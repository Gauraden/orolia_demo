#include "collector.hpp"

Collector::Collector() {
}

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

bool Collector::Begin() {
  return _source->OccupySource();
}

bool Collector::FetchAllRecords() {
  VoidDataSource::Record rec;
  bool push_ok = true;
  while (_source->GetRecord(&rec) && push_ok) {
    push_ok = _comp->PushRecord(rec);
  }
  return push_ok && _source->GetMessage().empty();
}

void Collector::End() {
  _source->ReleaseSource();
}

std::string Collector::GetErrorMessage() const {
  auto kSrcMessage  = _source->GetMessage();
  auto kCompMessage = _comp->GetMessage();
  if (not kSrcMessage.empty()) {
    return kSrcMessage;   
  }
  return kCompMessage;
}