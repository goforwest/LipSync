#ifndef _LSSERIALTXQUEUE_H
#define _LSSERIALTXQUEUE_H

#include <stddef.h>

// Fixed-capacity, allocation-free queue for newline-framed serial output.
// The consumer may write a line in several USB-FIFO-sized chunks. Once any
// prefix of the head line has been consumed, that line is protected until
// its newline is consumed: request prioritization and overflow may discard
// stale WHOLE lines, but can never leave a transmitted prefix to concatenate
// with an unrelated response.
template <size_t Capacity>
class LSSerialTxQueue {
  public:
    LSSerialTxQueue() { clear(); }

    void clear() {
      _head = 0;
      _tail = 0;
      _count = 0;
      _lineInProgress = false;
    }

    size_t size() const { return _count; }
    size_t freeSpace() const { return Capacity - _count; }
    bool lineInProgress() const { return _lineInProgress; }

    // Drop stale queued output before handling a new request. If part of the
    // head line is already on the wire, preserve exactly its unsent remainder
    // through the newline so the host still receives a valid complete frame.
    void prioritizeRequest() {
      if (!_lineInProgress) {
        clear();
        return;
      }
      preserveHeadLineOnly();
    }

    bool enqueueLine(const char* line, size_t length) {
      if (line == nullptr) return false;
      const size_t needed = length + 1;  // terminating newline
      if (needed >= Capacity) return false;

      while (freeSpace() < needed) {
        if (_lineInProgress) {
          // We cannot remove the partly-transmitted head. Discard every stale
          // line behind it in one operation; its small remainder plus the new
          // line must fit, or this new line is refused without corruption.
          preserveHeadLineOnly();
          if (freeSpace() < needed) return false;
        } else if (!dropOldestWholeLine()) {
          return false;
        }
      }

      for (size_t i = 0; i < length; ++i) pushByte(line[i]);
      pushByte('\n');
      return true;
    }

    const char* readPointer() const {
      return _count == 0 ? nullptr : &_buffer[_tail];
    }

    // Return one contiguous write chunk, never crossing a newline. Limiting
    // a service call to one line keeps the in-progress invariant exact even
    // when the USB FIFO has room for several frames.
    size_t nextChunkSize(size_t maxLength) const {
      if (_count == 0 || maxLength == 0) return 0;
      size_t chunk = _count;
      const size_t contiguous = Capacity - _tail;
      if (chunk > contiguous) chunk = contiguous;
      if (chunk > maxLength) chunk = maxLength;
      for (size_t i = 0; i < chunk; ++i) {
        if (_buffer[_tail + i] == '\n') return i + 1;
      }
      return chunk;
    }

    void consume(size_t length) {
      if (length > _count) length = _count;
      for (size_t i = 0; i < length; ++i) {
        const char consumed = _buffer[_tail];
        _tail = (_tail + 1) % Capacity;
        _count--;
        _lineInProgress = (consumed != '\n');
      }
      if (_count == 0) {
        _head = _tail;
        _lineInProgress = false;
      }
    }

  private:
    char _buffer[Capacity];
    size_t _head = 0;
    size_t _tail = 0;
    size_t _count = 0;
    bool _lineInProgress = false;

    void pushByte(char value) {
      _buffer[_head] = value;
      _head = (_head + 1) % Capacity;
      _count++;
    }

    bool dropOldestWholeLine() {
      if (_count == 0 || _lineInProgress) return false;
      while (_count > 0) {
        const char dropped = _buffer[_tail];
        _tail = (_tail + 1) % Capacity;
        _count--;
        if (dropped == '\n') break;
      }
      if (_count == 0) _head = _tail;
      return true;
    }

    void preserveHeadLineOnly() {
      if (_count == 0) return;
      size_t keep = 0;
      size_t index = _tail;
      while (keep < _count) {
        const char value = _buffer[index];
        keep++;
        index = (index + 1) % Capacity;
        if (value == '\n') break;
      }
      // Every enqueue appends a newline. If invariants are ever violated,
      // preserve everything rather than manufacturing a truncated frame.
      if (keep == _count && _buffer[(index + Capacity - 1) % Capacity] != '\n') return;
      _count = keep;
      _head = index;
    }
};

#endif
