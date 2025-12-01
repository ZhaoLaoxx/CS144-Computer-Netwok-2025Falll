#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <functional>
#include <queue>
#include <deque>

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ), senderQue(), windowSize(1), seqIdx(0), isSynSent(false), emptyStr(), times(0), RTOMs(initial_RTO_ms),
      consecutiveRetransmissions(0), isFinSent(false), byteBuffered(0), maxAckNo(0), ackSeqIdx(0), isRST(false)
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive retransmissions have happened?
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

  // add
  inline void reFreshTick() {
    times = 0;
  };

private:
  Reader& reader() { return input_.reader(); }
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  // add
  std::deque<TCPSenderMessage> senderQue;
  uint64_t windowSize;
  // absolute first unPushed sequence idx
  uint64_t seqIdx;
  bool isSynSent;
  const std::string emptyStr;
  uint64_t times;
  uint64_t RTOMs;
  uint64_t consecutiveRetransmissions;
  bool isFinSent;
  uint64_t byteBuffered;
  uint64_t maxAckNo;
  // absolute first unAck sequence idx
  uint64_t ackSeqIdx;
  bool isRST;
};
