#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <list>
#include "data_source.hpp"

/** UTF8
Разбор алгортма сжатия записей по шагам.
Важно следить за равномерностью "ёмкости" записей в буфере!
Необходимо равномерно распределять записи, т.е. при объединении
нескольких записей надо проверять количество значений помещённых 
в одну запись. Во всех записях буфера должно быть примерно одинаковое
количество значений (_rec_capacity), тогда графики будут точными.
[1 ,  ][2 ,  ][3 ,  ][4 ,  ][5 ,  ] <- 6  | _rec_capacity = 1
^
[1 , 2][3 ,  ][4 ,  ][5 ,  ][6 ,  ] <- 7  | _rec_capacity = 2
       ^
[1 , 2][3 , 4][5 ,  ][6 ,  ][7 ,  ] <- 8
              ^
[1 , 2][3 , 4][5 , 6][7 ,  ][8 ,  ] <- 9
                     ^
[1 , 2][3 , 4][5 , 6][7 , 8][9 ,  ] <- 10
                            ^
[1 , 2][3 , 4][5 , 6][7 , 8][9 ,10] <- 11
^
[1 , 4][5 , 6][7 , 8][9 ,10][11,  ] <- 12 | _rec_capacity = 4
       ^
[1 , 4][5 , 8][9 ,10][11,  ][12,  ] <- 13
              ^
[1 , 4][5 , 8][9 ,11][12,  ][13,  ] <- 14
              ^
[1 , 4][5 , 8][9 ,12][13,  ][14,  ] <- 15
                     ^
[1 , 4][5 , 8][9 ,12][13,14][15,  ] <- 16
                     ^
[1 , 4][5 , 8][9 ,12][13,15][16,  ] <- 17
                     ^
[1 , 4][5 , 8][9 ,12][13,16][17,  ] <- 18
                     ^
[1 , 4][5 , 8][9 ,12][13,16][17,18] <- 19
                            ^
[1 , 4][5 , 8][9 ,12][13,16][17,19] <- 20
                            ^
[1 , 4][5 , 8][9 ,12][13,16][17,20] <- 21
^                           
[1 , 8][9 ,12][13,16][17,20][21,  ] <- 22 | _rec_capacity = 8
       ^
**/
/**
 * Class for compressing big amount of records into small buffer.
 */
class Compressor {
  public:
    typedef std::shared_ptr<Compressor> ShrPtr;
    typedef std::pair<double, double>   Range;
    /**
     * Class for representation of compressed record.
     * It has fields:
     * - time  : range of time labels
     * - value : range of values into the time interval
     * - amount: amount of records into the time interval
     */
    class Record {
      public:
        typedef std::list<Record> List;

        Record(double time, double value);
        Record(const Range &time, const Range &value);
        Record(const VoidDataSource::Record &rec);
        /**
         * Method for merging two records into one.
         * For better understanding please look at 
         * (tests/test_compressor : CompressorRecordMergeTest)
         * @param src reference to the record that will be 
         *            merged with current. Ranges: time, values;
         *            will be extended by "src".
         * @return true if merging was finished.              
         */
        bool MergeWith(const Record &src);

        Range    time;
        Range    value;
        uint32_t amount;
    };

    Compressor(uint32_t max_size);
    virtual ~Compressor();
    /**
     * Method for pushing recods into the buffer. During this, old records
     * will be merged for getting free space, if list size has riched limit.
     * For better understanding please look at
     * (tests/test_compressor : CompressorPushTest) 
     * @param new_rec reference for new record, wich we want to add to 
     *                the buffer;
     * @return true if pushing was finished.
     */
    virtual bool PushRecord(Record &&new_rec);
    /**
     * Method for calculating scales: time, values;
     * @param rec reference for record
     */
    void PrecalculateScales(const Record &rec);
    const std::string& GetMessage() const;
    /**
     * Method for getting ratios of record ranges projected on scales.
     * @param rec record that we want to projet on scales
     * @param out values of ratios, it is an output parameter
     * @return amount of ratios in output
     */
    uint8_t CastRecordToScales(const Record &rec, Range out[2]) const;
    const Record::List& GetRecords() const;
    double GetTimeScaleLen() const;
    double GetValueScaleLen() const;
  protected:
    void SetMessage(const std::string &msg);
  private:
    size_t                 _pushed_records;
    uint32_t               _rec_capacity;
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