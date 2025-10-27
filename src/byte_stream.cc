#include "byte_stream.hh"
#include "debug.hh"
#include <string.h>

using namespace std;

ByteStream::ByteStream(uint64_t capacity) : capacity_(capacity), error_(false), str(make_unique<char[]>(capacity_)),
 readIdx(0), writeIdx(0), isWriterClosed(false), pushBytes(0), popBytes(0), bufferBytes(0), str_() {}

// Push data to stream, but only as much as available capacity allows.
void Writer::push(string data)
{
  debug("pushBytes {} data.size() {}", pushBytes, data.size());
  uint64_t cnt = min(data.size(), capacity_ - bufferBytes);
  if (cnt == 0) return;
  if (capacity_ - writeIdx < cnt) {
    // 分两段
    uint64_t temp1 = capacity_ - writeIdx;
    uint64_t temp2 = cnt - temp1;
    memcpy(str.get() + writeIdx, data.data(), temp1);
    writeIdx = 0;
    memcpy(str.get() + writeIdx, data.data() + temp1, temp2);
    writeIdx += temp2;
  } else {
    memcpy(str.get() + writeIdx, data.data(), cnt);
    writeIdx += cnt;
  }
  bufferBytes += cnt;
  pushBytes += cnt;
}

// Signal that the stream has reached its ending. Nothing more will be written.
void Writer::close()
{
  // debug("Writer::close()");
  isWriterClosed = true;
}

// Has the stream been closed?
bool Writer::is_closed() const
{
  // debug("Writer::is_closed()");
  return isWriterClosed;
}

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const
{
  // debug("Writer::available_capacity: {}", capacity_ - bufferBytes);
  return capacity_ - bufferBytes;
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const
{
  // debug("Writer::bytes_pushed: {}", pushBytes);
  return pushBytes;
}

// Peek at the next bytes in the buffer -- ideally as many as possible.
// It's not required to return a string_view of the *whole* buffer, but
// if the peeked string_view is only one byte at a time, it will probably force
// the caller to do a lot of extra work.
string_view Reader::peek() const
{
  // debug("Reader::peek()");
  if (readIdx + bufferBytes >= capacity_) {
    return string_view(str.get() + readIdx, capacity_ - readIdx);
  }
  return string_view(str.get() + readIdx, bufferBytes);
}

// Remove `len` bytes from the buffer.
void Reader::pop(uint64_t len)
{
  // debug("Reader::pop({})", len);
  // std::lock_guard<std::mutex> guard(*lock.get());
  len = min(len, bufferBytes);
  bufferBytes -= len;
  readIdx += len;
  popBytes += len;
  if (readIdx >= capacity_) readIdx -= capacity_;
}

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const
{
  // debug("Reader::is_finished() isWriterClosed: {} bufferBytes: {}", isWriterClosed, bufferBytes);
  if (isWriterClosed && bufferBytes == 0) {
    return true;
  }
  return false;
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const
{
  // debug("Reader::bytes_buffered() {}", bufferBytes);
  return bufferBytes;
}

// Total number of bytes cumulatively popped from stream
uint64_t Reader::bytes_popped() const
{
  // debug("Reader::bytes_popped() {}", popBytes);
  return popBytes;
}
