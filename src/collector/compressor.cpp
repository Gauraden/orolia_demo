#include "compressor.hpp"
#include <cmath>
#include <boost/lexical_cast.hpp>
#include <iostream>

std::ostream& operator<< (std::ostream &s, const Compressor::Range &rng) {
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
    : _max_size(max_size),
      _pushed_records(0),
      _time_scale(std::nan(""), std::nan("")),
      _value_scale(std::nan(""), std::nan("")) {
}

Compressor::~Compressor() {
}

void Compressor::PrecalculateScales(const Record &rec) {
  ++_pushed_records;
  if (std::isnan(_time_scale.first) || 
      rec.time.first < _time_scale.first) {
    _time_scale.first = rec.time.first;
  }
  if (std::isnan(_time_scale.second) ||
      rec.time.first > _time_scale.second) {
    _time_scale.second = rec.time.first;
  }
  if (std::isnan(_value_scale.first) || 
      rec.value.first < _value_scale.first) {
    _value_scale.first = rec.value.first;
  }
  if (std::isnan(_value_scale.second) || 
      rec.value.first > _value_scale.second) {
    _value_scale.second = rec.value.first;
  }
}

bool Compressor::PushRecord(Record &&new_rec) {
  PrecalculateScales(new_rec);
  const auto kAmountOfRecs = _records.size();
  if (kAmountOfRecs < _max_size) {
    _records.push_back(new_rec);
    if (kAmountOfRecs == 1) {
      _record_it = _records.begin();
    }
    return true;
  }
  bool was_merged = false;
  auto prev_rec   = *_record_it;
  while (std::isnan(_record_it->time.first) ||
         std::isnan(std::next(_record_it)->time.first)) {
    prev_rec = *(++_record_it);
  }
  _record_it = _records.erase(_record_it);
  if (_record_it != _records.end()) {
    was_merged = _record_it->MergeWith(prev_rec);
    ++_record_it;
    if (_record_it == _records.end()) {
      _record_it = _records.begin();
    }
  } else {
    was_merged = new_rec.MergeWith(prev_rec);
    _record_it = _records.begin();
  }
  if (not was_merged) {
    SetMessage("Failed to push record! Record #" 
      + boost::lexical_cast<std::string>(_pushed_records)
    );
    return false;
  }
  _records.push_back(new_rec);
  return true;
}

uint8_t Compressor::CastRecordToScales(const Record &rec, Range out[2]) const {
  const double kTimeScaleLen  = _time_scale.second - _time_scale.first;
  const double kValueScaleLen = _value_scale.second - _value_scale.first;
  if (kTimeScaleLen == 0.0 || kValueScaleLen == 0.0 ||
      std::isnan(rec.time.first)) {
    return 0;
  }
  out[0] = Range(
    (rec.time.first - _time_scale.first) / kTimeScaleLen,
    (rec.value.first - _value_scale.first) / kValueScaleLen
  );
  if (std::isnan(rec.time.second)) {
    return 1;
  }
  out[1] = Range(
    (rec.time.second - _time_scale.first) / kTimeScaleLen,
    (rec.value.second - _value_scale.first) / kValueScaleLen
  );
  return 2;
}

const Compressor::Record::List& Compressor::GetRecords() const {
  return _records;
}

double Compressor::GetTimeScaleLen() const {
  return (_time_scale.second - _time_scale.first);
}

double Compressor::GetValueScaleLen() const {
  return (_value_scale.second - _time_scale.first);
}

const std::string& Compressor::GetMessage() const {
  return _message;
}

void Compressor::SetMessage(const std::string &msg) {
  _message = msg;
}