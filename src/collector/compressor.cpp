#include "compressor.hpp"
#include <cmath>
#include <iostream>
#include <boost/lexical_cast.hpp>

std::ostream& operator<< (std::ostream &s, const Compressor::Record::Range &rng) {
  s << std::fixed << rng.first << " - " << std::fixed << rng.second;
  return s;
}

std::ostream& operator<< (std::ostream &s, const Compressor::Record &rec) {
  s << "time : " << rec.time  << "; "
    << "value: " << rec.value;
  return s;
}

// class Compressor::Record
Compressor::Record::Record(double time, double value)
    : time(time, std::nan("")),
      value(value, std::nan("")) {
}

Compressor::Record::Record(const Range &time, const Range &value)
    : time(time),
      value(value) {
}

Compressor::Record::Record(const VoidDataSource::Record &rec)
    : time(rec.time, std::nan("")),
      value(rec.value, std::nan("")) {
}

static
void SwapIfGreater(double &f, double &s) {
  if (s > f || std::isnan(f)) {
    std::swap(f, s);
  }
}

bool Compressor::Record::MergeWith(const Record &src) {
  // merging old record (src) with new (this)
  if (src.time.first > time.first ||
      (not std::isnan(src.time.second) && src.time.second > time.first)
     ) {
    return false;
  }
  SwapIfGreater(time.second, time.first);
  time.first = src.time.first;
  if (src.value.first < value.first) {
    SwapIfGreater(value.second, value.first);
    value.first = src.value.first;
  }
  double t_sec = src.value.second;
  if (std::isnan(t_sec)) {
    t_sec = src.value.first;
  }
  SwapIfGreater(value.second, t_sec);
  SwapIfGreater(value.second, value.first);
  return true;
}
// class Compressor
Compressor::Compressor(uint32_t max_size)
    : max_size(max_size),
      _pushed_records(0) {
}

Compressor::~Compressor() {
}

bool Compressor::PushRecord(Record &&new_rec) {
  const auto kAmountOfRecs = records.size();
  if (kAmountOfRecs < max_size) {
    records.push_back(new_rec);
    if (kAmountOfRecs == 1) {
      record_it = records.begin();
    }
    ++_pushed_records;
    return true;
  }
  bool was_merged = false;
  const auto kPrevRec = *record_it;
  record_it = records.erase(record_it);
  if (record_it != records.end()) {
    was_merged = record_it->MergeWith(kPrevRec);
    ++record_it;
    if (record_it == records.end()) {
      record_it = records.begin();
    }
  } else {
    was_merged = new_rec.MergeWith(kPrevRec);
    record_it = records.begin();
  }
  if (not was_merged) {
    SetMessage("Failed to push record! Record #" 
      + boost::lexical_cast<std::string>(_pushed_records)
    );
    return false;
  }
  records.push_back(new_rec);
  ++_pushed_records;
  return true;
}

const std::string& Compressor::GetMessage() const {
  return _message;
}

void Compressor::SetMessage(const std::string &msg) {
  _message = msg;
}