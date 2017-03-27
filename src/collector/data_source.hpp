#ifndef DATA_SOURCE_HPP
#define DATA_SOURCE_HPP

#include <memory>
#include <string>
#include <fstream>

class VoidDataSource {
  public:
    typedef std::shared_ptr<VoidDataSource> ShrPtr;

    struct Header;
    struct Record;

    VoidDataSource();
    virtual ~VoidDataSource();
    virtual bool OccupySource();
    virtual void ReleaseSource();

    const Header& GetHeader();
    bool GetRecord(Record *out);
    bool IsAtTheEnd() const;
    const std::string& GetMessage() const;
  protected:
    virtual int16_t GetLine(char *line, uint8_t max_len) = 0;
    void SetMessage(const std::string &msg);
  private:
    static const uint8_t kLineSize = 255;

    bool         _occupied;
    bool         _end_of_source;
    Header      *_header;
    char         _line[kLineSize];
    std::string  _message;
    uint32_t     _rows_amount;
    double       _prev_time_label;
};

struct VoidDataSource::Header {
  struct Creator {
    Creator();
    bool IsValid() const;

    std::string name;
    std::string version;
  };

  Header();
  bool IsValid() const;

  Creator     created_by;
  std::string type_of_measurement;
  std::string time_of_start;
  std::string measuring_time;
  std::string input_a;
  std::string input_b;
  bool        ext_arm;
  bool        hold_off;
  bool        single;
  bool        filter;
  bool        common;
  std::string ref_osc;
  bool        statistics;          
};

struct VoidDataSource::Record {
  Record();
  Record(double time, double value);
  double time;
  double value;
};

class FileDataSource : public VoidDataSource {
  public:
    FileDataSource(const std::string &path);
  protected:
    virtual bool OccupySource();
    virtual int16_t GetLine(char *line, uint8_t max_len);
    virtual void ReleaseSource();
  private:
    std::string  _file;
    std::fstream _source;
};
#endif