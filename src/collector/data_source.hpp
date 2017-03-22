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
  protected:
    virtual bool GetLine(char *line, size_t max_len) = 0;   

    static const size_t kLineSize = 256;

    bool    _occupied;
    Header *_header;
    char    _line[kLineSize];
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
  double time;
  double value;
};

class FileDataSource : public VoidDataSource {
  public:
    FileDataSource(const std::string &path);
    virtual ~FileDataSource();
    virtual bool OccupySource();
    virtual bool GetLine(char *line, size_t max_len);
    virtual void ReleaseSource();
  private:
    std::string  _file;
    std::fstream _source;
};