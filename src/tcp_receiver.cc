#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  debug("TCPReceiver::receive() called");
  if (message.RST) rstFlag = true, reassembler_.reader().set_error();
  if (message.SYN == true) {
    zeroPoint = message.seqno;
    hasZeroPoint = true;
    leftPointer = 1;
    // 特判：带上SYN的Seg的payload有可能有数据，例如，receive message: (seqno=Wrap32<1434529794> +SYN payload="Hello, CS144!")  
    if (!message.payload.empty()) {
        reassembler_.insert(0, message.payload, false);
        leftPointer = reassembler_.getFirstUnPopIdx() + 1;
    }
    // 特判：SYN 和 FIN 同时出现 
    if (message.FIN == true) {
      reassembler_.insert(0, message.payload, true);
      leftPointer += 1;
    }
    return;
  }
  if (!hasZeroPoint) return;
  if (message.FIN) isFinished = true;
  bool isFinalSeg = (message.FIN == true);
  reassembler_.insert(message.seqno.unwrap(zeroPoint.value(), leftPointer) - 1, message.payload, isFinalSeg);
  leftPointer = reassembler_.getFirstUnPopIdx() + 1;
  if (isFinished && reassembler_.count_bytes_pending() == 0) {
    leftPointer += 1;
  }
  debug("TCPReceiver::receive() called absoluteIdx: {}", message.seqno.unwrap(zeroPoint.value(), leftPointer));
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  debug( "TCPReceiver::send() called" );
  uint16_t windowSize = reassembler_.getCapacity() - reassembler_.reader().bytes_buffered() > UINT16_MAX
   ? UINT16_MAX : reassembler_.getCapacity() - reassembler_.reader().bytes_buffered();
  if (!hasZeroPoint) {
    return {std::nullopt, windowSize, reassembler_.reader().has_error()};
  }
  return {Wrap32::wrap(leftPointer, zeroPoint.value()), windowSize, reassembler_.reader().has_error()};
}
