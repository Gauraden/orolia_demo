#include <list>
#include "data_source.hpp"

class Compressor {
  public:
    typedef std::shared_ptr<Compressor> ShrPtr;

    class Record {
      public:
        typedef std::list<Record>         List;
        typedef std::pair<double, double> Range;

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
    const std::string& GetMessage() const;

    const uint32_t         max_size;
    Record::List           records;
    Record::List::iterator record_it;
  protected:
    void SetMessage(const std::string &msg);
  private:
    size_t      _pushed_records;
    std::string _message;
};

std::ostream& operator<< (std::ostream &s, const Compressor::Record::Range &rng);
std::ostream& operator<< (std::ostream &s, const Compressor::Record &rec);