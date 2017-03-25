#include <cmath>
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

void Collector::RegisterMessage(const std::string &msg) {
  if (not msg.empty()) {
    _messages.emplace_back(msg);
  }
}

bool Collector::Begin() {
  if (not _source->OccupySource()) {
    RegisterMessage(_source->GetMessage());    
    return false;
  }
  return true;
}

bool Collector::FetchAllRecords() {
  VoidDataSource::Record rec;
  bool push_ok = true;
  while (not _source->IsAtTheEnd() && push_ok) {
    if (_source->GetRecord(&rec)) {
      push_ok = _comp->PushRecord(rec);
    } else {
      _comp->PushRecord({std::nan(""), std::nan("")});
      RegisterMessage(_source->GetMessage());
    }
  }
  if (not push_ok) {
    RegisterMessage(_comp->GetMessage());
  }
  return push_ok;
}

void Collector::End() {
  _source->ReleaseSource();
}

const Collector::Messages& Collector::GetMessages() const {
  return _messages;
}