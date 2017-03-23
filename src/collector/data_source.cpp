#include "data_source.hpp"
#include <iostream>
#include <list>
#include <regex>
#include <cmath>

// class VoidDataSource
VoidDataSource::VoidDataSource()
    : _occupied(false),
      _header(new Header()) {
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
  list->emplace_back("\\w{3,3} \\w{3,3} \\d{1,2} \\d{1,2}:\\d{1,2}:\\d{1,2} \\d{4,4}",
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
  while (GetLine(_line, kLineSize)) {
    if (_line[0] != '#') {
      break;
    }
    std::smatch m;
    for (auto fit = fields.begin(); fit != fields.end(); ++fit) {
      std::string t_str(_line);
      if (std::regex_search(t_str, m, fit->regex)) {
        fit->handler(m, _header);
        fields.erase(fit);
        break;
      }
    }
  }
  return (fields.size() == 0);
}

void VoidDataSource::ReleaseSource() {
  _occupied = false;
}

const VoidDataSource::Header& VoidDataSource::GetHeader() {
  return *_header;
}

bool VoidDataSource::GetRecord(Record *out) {
  if (not GetLine(_line, kLineSize) || _line[0] == 0) {
    return false;
  }
  std::string::size_type next;
  try {
    out->time  = std::stod(_line, &next);
    out->value = std::stod(_line + next);
  } catch (...) {
    return false;
  }
  return true;
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
    return false;
  }
  return VoidDataSource::OccupySource();
}

bool FileDataSource::GetLine(char *line, size_t max_len) {
  const bool kIsGood = _source.good();
  if (kIsGood) {
    _source.getline(line, max_len);
  }
  return kIsGood;
}

void FileDataSource::ReleaseSource() {
  _source.close();
  VoidDataSource::ReleaseSource();  
}