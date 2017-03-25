#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <list>
#include "data_source.hpp"

class Compressor {
  public:
    typedef std::shared_ptr<Compressor> ShrPtr;
    typedef std::pair<double, double>   Range;

    class Record {
      public:
        typedef std::list<Record> List;

        Record(double time, double value);
        Record(const Range &time, const Range &value);
        Record(const VoidDataSource::Record &rec);
        bool MergeWith(const Record &src);

        Range time;
        Range value;
    };

    Compressor(uint32_t max_size);
    virtual ~Compressor();
    virtual bool PushRecord(Record &&new_rec);
    void PrecalculateScales(const Record &rec);
    const std::string& GetMessage() const;
    uint8_t CastRecordToScales(const Record &rec, Range out[2]) const;
    const Record::List& GetRecords() const;
    double GetTimeScaleLen() const;
    double GetValueScaleLen() const;
  protected:
    void SetMessage(const std::string &msg);
  private:
    size_t                 _pushed_records;
    std::string            _message;
    Range                  _time_scale;
    Range                  _value_scale;
    const uint32_t         _max_size;
    Record::List           _records;
    Record::List::iterator _record_it;
};

std::ostream& operator<< (std::ostream &s, const Compressor::Range &rng);
std::ostream& operator<< (std::ostream &s, const Compressor::Record &rec);
#endif