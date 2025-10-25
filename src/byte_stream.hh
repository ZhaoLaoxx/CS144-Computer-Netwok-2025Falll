#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <iostream>

class Reader;
class Writer;

class ByteStream
{
public:
  explicit ByteStream( uint64_t capacity );

  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;

  void set_error() { error_ = true; };       // Signal that the stream suffered an error.
  bool has_error() const { return error_; }; // Has the stream had an error?

  uint64_t getCapacity() const { return capacity_; }

protected:
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.
  uint64_t capacity_;
  bool error_ {};
  std::shared_ptr<char[]> str;
  uint64_t readIdx;
  uint64_t writeIdx;
  // 不能直接 std::mutex lock，因为 std::mutex 不可拷贝（删除delete了构造拷贝函数）
  // 使用 shared_ptr 就可以多个智能指针管理一个 公共的互斥锁
  std::shared_ptr<std::mutex> lock;
  bool isWriterClosed;
  uint64_t pushBytes;
  uint64_t popBytes;
  uint64_t bufferBytes;
  std::shared_ptr<std::mutex> readerLock;
  std::shared_ptr<std::mutex> writerLock;
};

class Writer : public ByteStream
{
public:
  void push( std::string data ); // Push data to stream, but only as much as available capacity allows.
  void close();                  // Signal that the stream has reached its ending. Nothing more will be written.

  bool is_closed() const;              // Has the stream been closed?
  uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now?
  uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream
};

class Reader : public ByteStream
{
public:
  std::string_view peek() const; // Peek at the next bytes in the buffer -- ideally as many as possible.
  void pop( uint64_t len );      // Remove `len` bytes from the buffer.

  bool is_finished() const;        // Is the stream finished (closed and fully popped)?
  uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped)
  uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream
};

/*
 * read: A (provided) helper function thats peeks and pops up to `max_len` bytes
 * from a ByteStream Reader into a string;
 */
void read( Reader& reader, uint64_t max_len, std::string& out );
