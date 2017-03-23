#include "collector.hpp"

void Collector::UseCompressor(Compressor *ptr) {
  _comp.reset(ptr);
}

void Collector::UseDataSource(VoidDataSource *ptr) {
  _source.reset(ptr);
}

Compressor::ShrPtr Collector::GetCompressor() const {
  return _comp;
}

bool Collector::GetDataHeader(VoidDataSource::Header *out) const {
  if (not _source || out == 0) {
    return false;
  }
  *out = _source->GetHeader();
  return true;
}