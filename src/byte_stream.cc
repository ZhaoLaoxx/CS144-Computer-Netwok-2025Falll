#include "byte_stream.hh"
#include "debug.hh"

using namespace std;

ByteStream::ByteStream(uint64_t capacity) : capacity_(capacity), error_(false), str(make_unique<char[]>(capacity_)),
 readIdx(0), writeIdx(0), lock(make_shared<std::mutex>()), isWriterClosed(false),
  pushBytes(0), popBytes(0), bufferBytes(0),
   readerLock(make_shared<std::mutex>()), writerLock(make_shared<std::mutex>()) {}

// Push data to stream, but only as much as available capacity allows.
void Writer::push(string data)
{
  // std::lock_guard<std::mutex> guard(*lock.get());
  int nums = 0;
  // debug("pushBytes {} data.size() {}", pushBytes, data.size());
  for(auto cr : data) {
    if (bufferBytes == capacity_) break;
    str.get()[writeIdx ++] = cr;
    pushBytes ++, bufferBytes ++, nums ++;
    if (writeIdx >= capacity_) writeIdx -= capacity_;
  }
  // debug("Writer::Push success {} pushBytes {}", nums, pushBytes);
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
  // std::lock_guard<std::mutex> guard(*lock.get());
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
  return string_view(str.get() + readIdx, 1);
}

// Remove `len` bytes from the buffer.
void Reader::pop(uint64_t len)
{
  // debug("Reader::pop({})", len);
  // std::lock_guard<std::mutex> guard(*lock.get());
  for(uint32_t i = 0; i < len && bufferBytes > 0; readIdx ++, i ++, popBytes ++, bufferBytes -= 1) {
    if (readIdx >= capacity_) readIdx -= capacity_;
  }
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
