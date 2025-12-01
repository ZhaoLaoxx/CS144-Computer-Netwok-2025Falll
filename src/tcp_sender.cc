#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

# define IS_SYN  true
# define NOT_SYN false
# define IS_FIN true
# define NOT_FIN false
# define NOT_RST false

using namespace std;

// How many sequence numbers are outstanding?
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  debug( "sequence_numbers_in_flight() called" );
  return seqIdx - ackSeqIdx;
}

// How many consecutive retransmissions have happened?
uint64_t TCPSender::consecutive_retransmissions() const
{
  debug( "consecutive_retransmissions() called" );
  return consecutiveRetransmissions;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  debug( "implemented push() called" );

  if (isSynSent && !reader().bytes_buffered()) {
    if ((reader().is_finished() && isFinSent) || (!reader().is_finished())) {
      debug("TCPSender::push no bytes send");
      return;
    }
  }

  // check SYN
  bool isRst = false, hasSynFlag = false, hasFinFlag = false;
  if (!isSynSent) {
    isSynSent = hasSynFlag = true;
  }
  if (!isRst && reader().has_error()) isRst = true;

  string_view strView = reader().peek();
  uint64_t trueWindowSize = max(windowSize, 1UL);
  uint64_t strLen = min({strView.size(), trueWindowSize - byteBuffered, TCPConfig::MAX_PAYLOAD_SIZE});
  if (strLen + hasSynFlag > TCPConfig::MAX_PAYLOAD_SIZE || strLen + hasSynFlag > trueWindowSize) {
    strLen -= 1;
  }
  string payload(strView.substr(0, strLen));
  reader().pop(strLen);

  TCPSenderMessage plMsg(Wrap32::wrap(seqIdx, isn_), hasSynFlag, std::move(payload), hasFinFlag, isRst);
  // check FIN
  if(reader().is_finished()) {
    isFinSent = hasFinFlag = true;
    plMsg.setFIN();
    if (byteBuffered + plMsg.sequence_length() > trueWindowSize) {
      plMsg.clearFIN();
      isFinSent = false;
    }
  }

  if (!plMsg.sequence_length()) {
    // 空消息直接跳过
    debug("TCPSender::push emptyMsg return");
    return;
  }
  debug("TCPSender::push payload: {} StrSize: {} sequenceLen: {} seqIdx: {} rawIdx: {}", payload, strLen, plMsg.sequence_length(), seqIdx, Wrap32::wrap(seqIdx, isn_).getRawValue());
  transmit(plMsg);
  seqIdx += plMsg.sequence_length();
  byteBuffered += plMsg.sequence_length();
  senderQue.emplace_back(plMsg);

  // 如果是big windowSize，则需要多次push
  if (windowSize > byteBuffered) push(transmit);
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  debug("make_empty_message() called");

  return {Wrap32::wrap(seqIdx, isn_), NOT_SYN, emptyStr, NOT_FIN, reader().has_error()};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  debug( "receive() called" );
  windowSize = msg.window_size;
  consecutiveRetransmissions = 0;
  if (msg.RST) {
    reader().set_error();
    isRST = true;
  }
  if (msg.ackno) {
    uint64_t receiveAckIdx = msg.ackno.value().unwrap(isn_, ackSeqIdx);

    if(!senderQue.empty()) {
      // 超出右窗口最大距离，直接判定为不合法的ack
      if (receiveAckIdx > ackSeqIdx + byteBuffered) {
        return;
      }
    } else return;

    debug("receive senderQueSize {}", senderQue.size());
    while(!senderQue.empty()) {
      if(receiveAckIdx >= senderQue.front().seqno.unwrap(isn_, ackSeqIdx) + senderQue.front().sequence_length()) {
        ackSeqIdx = senderQue.front().seqno.unwrap(isn_, ackSeqIdx) + senderQue.front().sequence_length();
        byteBuffered -= senderQue.front().sequence_length();
        senderQue.pop_front();
      } else break;
    }

    // 重传机制-RECEIVE规则（当前该ackno大于以往任何ackno）
    // 1. RTO重置为初始值
    // 2. 若sender仍有未确认的数据，则重启定时器，设定其在当前RTO值对应的毫秒数后到期；补充，如果没有数据，则关闭tick
    // 3. 将连续重传次数（consecutiveRetransmissions）置0
    if(receiveAckIdx > maxAckNo) {
      RTOMs = initial_RTO_ms_;
      if (!senderQue.empty()) {
        reFreshTick();
      }
      maxAckNo = receiveAckIdx;
    }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if(senderQue.empty()) return;
  times += ms_since_last_tick;
  debug("tick({}) called {} {} {}", ms_since_last_tick, senderQue.size(), times, RTOMs);
  if (times >= RTOMs) {
    TCPSenderMessage tcp = senderQue.front();
    transmit(tcp);
    if (windowSize != 0) {
      // 重传机制：如果窗口非零
      // 1. 连续重传次数（consecutiveRetransmissions）递增
      // 2. RTO翻倍
      consecutiveRetransmissions += 1;
      RTOMs <<= 1;
    }
    times = 0;
  }
}
