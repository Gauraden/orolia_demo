#include "data_source.hpp"
#include <list>
#include <regex>
#include <cmath>
#include <boost/lexical_cast.hpp>

// class VoidDataSource
VoidDataSource::VoidDataSource()
    : _occupied(false),
      _end_of_source(false),
      _header(new Header()),
      _rows_amount(0),
      _prev_time_label(std::nan("")) {
}

VoidDataSource::~VoidDataSource() {
  delete _header;
  if (_occupied) {
    ReleaseSource();
  }
}

struct Field {
  typedef void (*Handler)(const std::smatch &m, VoidDataSource::Header *out);
  typedef std::list<Field> List;

  Field(const char reg_str[], Handler handler)
      : regex(reg_str, std::regex_constants::icase),
        handler(handler) {
  }
  std::regex regex;
  Handler    handler;
};

static
bool GetLogicFieldVal(const std::string &val) {
  return val == "Off" ? false : true;  
}

static
void CreateFieldsHandlers(Field::List *list) {
  list->emplace_back("Pendulum Instruments AB, ([^\\W]+) V([0-9\\.]+)",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->created_by.name    = m[1];
      out->created_by.version = m[2];
  });
  list->emplace_back("FREQUENCY A",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->type_of_measurement = m[0];
    }
  );
  list->emplace_back("\\w{3,4} \\w{3,4} \\d{1,2} \\d{1,2}:\\d{1,2}:\\d{1,2} \\d{4,4}",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->time_of_start = m[0];
    }
  );
  list->emplace_back("Measuring time: (\\d+) (\\w+)(\\W+)Single: (\\w+)",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->measuring_time = (std::string)m[1] + " " + (std::string)m[2];
      out->single         = GetLogicFieldVal(m[4]);
    }
  );
  list->emplace_back("Input A: (.+?)(\\W+)Filter: (\\w+)",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->input_a = m[1];
      out->filter  = GetLogicFieldVal(m[3]);
    }
  );
  list->emplace_back("Input B: (.+?)(\\W+)Common: (\\w+)",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->input_b = m[1];
      out->common  = GetLogicFieldVal(m[3]);
    }
  );
  list->emplace_back("Ext.arm: (\\w+)(\\W+)Ref.osc: (\\w+)",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->ext_arm = GetLogicFieldVal(m[1]);
      out->ref_osc = m[3];
    }
  );
  list->emplace_back("Hold off: (\\w+)(\\W+)Statistics: (\\w+)",
    [](const std::smatch &m, VoidDataSource::Header *out) {
      out->hold_off   = GetLogicFieldVal(m[1]);
      out->statistics = GetLogicFieldVal(m[3]);
    }
  );
}

bool VoidDataSource::OccupySource() {
  _occupied = true;
  Field::List fields;
  CreateFieldsHandlers(&fields);
  // reading header
  while (fields.size() > 0 && GetLine(_line, kLineSize) >= 0) {
    if (_line[0] != '#') {
      break;
    }
    std::smatch m;
    for (auto fit = fields.begin(); fit != fields.end(); ++fit) {
      std::string t_str(_line);
      if (std::regex_search(t_str, m, fit->regex)) {
        fit->handler(m, _header);
        fields.erase(fit);
        ++_rows_amount;
        break;
      }
    }
  }
  if (fields.size() > 0) {
    SetMessage("Invalid header!");
    return false;
  }
  return true;
}

void VoidDataSource::ReleaseSource() {
  _occupied = false;
}

const VoidDataSource::Header& VoidDataSource::GetHeader() {
  return *_header;
}

bool VoidDataSource::IsAtTheEnd() const {
  return _end_of_source;
}

bool VoidDataSource::GetRecord(Record *out) {
  const auto kGetLen = GetLine(_line, kLineSize);
  if (kGetLen < 0) {
    _end_of_source = true;
    return false;
  }
  ++_rows_amount;
  std::string::size_type next;
  try {
    out->time  = std::stod(_line, &next);
    out->value = std::stod(_line + next);
  } catch (...) {
    if (kGetLen > 2) {
      SetMessage("Failed to parse line #"
        + boost::lexical_cast<std::string>(_rows_amount)
      );
    }
    return false;
  }
  if (not std::isnan(_prev_time_label) &&
      out->time < _prev_time_label) {
    SetMessage("Invalid time label at line #"
      + boost::lexical_cast<std::string>(_rows_amount)
    );
    return false;
  }
  _prev_time_label = out->time;
  return true;
}

const std::string& VoidDataSource::GetMessage() const {
  return _message;
}

void VoidDataSource::SetMessage(const std::string &msg) {
  _message = msg;
}
// class VoidDataSource::Header
VoidDataSource::Header::Creator::Creator() {
}

bool VoidDataSource::Header::Creator::IsValid() const {
  return (
    name.size() > 0 &&
    version.size() > 0
  );
}

VoidDataSource::Header::Header()
    : ext_arm(false),
      hold_off(false),
      single(false),
      filter(false),
      common(false),
      statistics(false) {
}

bool VoidDataSource::Header::IsValid() const {
  return (
    created_by.IsValid() &&
    type_of_measurement.size() > 0 &&
    time_of_start.size() > 0 &&
    measuring_time.size() > 0 &&
    input_a.size() > 0 &&
    input_b.size() > 0 &&
    ref_osc.size() > 0
  );
}
// class VoidDataSource::Record
VoidDataSource::Record::Record()
    : time(std::nan("")),
      value(std::nan("")) {
}
VoidDataSource::Record::Record(double time, double value)
    : time(time),
      value(value) {
}
// class FileDataSource
FileDataSource::FileDataSource(const std::string &path)
    : VoidDataSource(),
      _file(path) {
}

FileDataSource::~FileDataSource() {
}

bool FileDataSource::OccupySource() {
  _source.open(_file);
  if (not _source.is_open()) {
    SetMessage("Failed to open file: " + _file);
    return false;
  }
  return VoidDataSource::OccupySource();
}

int16_t FileDataSource::GetLine(char *line, uint8_t max_len) {
  if (_source.good()) {
    const auto kGCount = _source.getline(line, max_len).gcount();
    return kGCount;
  }
  return -1;
}

void FileDataSource::ReleaseSource() {
  _source.close();
  VoidDataSource::ReleaseSource();  
}