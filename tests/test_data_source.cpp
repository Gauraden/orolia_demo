#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>
#include "../src/collector/data_source.hpp"

struct DataSourceTestFixture {

    class TestSource: public VoidDataSource {
      public:
        TestSource(): VoidDataSource() {}
        virtual ~TestSource() {}
        virtual bool GetLine(char *line, size_t max_len) {
          const bool kIsGood = data.good();
          if (kIsGood) {
            data.getline(line, max_len);
          }
          return kIsGood;
        }
        std::stringstream data;
    };

    DataSourceTestFixture() {}
    ~DataSourceTestFixture() {}
};
// -----------------------------------------------------------------------------
// Инициализация набора тестов
BOOST_FIXTURE_TEST_SUITE(DataSourceTestSuite, DataSourceTestFixture)

BOOST_AUTO_TEST_CASE(VoidDataSourceInvalidHeaderTest) {
  VoidDataSource::Header hd;
  BOOST_CHECK(not hd.IsValid());
}

BOOST_AUTO_TEST_CASE(VoidDataSourceReadHeaderTest) {
  TestSource src;
  src.data
    << "# Pendulum Instruments AB, TimeView32 V1.01" << std::endl
    << "# FREqUENCY A" << std::endl
    << "# MON May 12 13:13:23 2003" << std::endl
    << "# Measuring time: 10 ms                       Single: Off" << std::endl
    << "# Input A: Auto, 1M., AC, X1, Pos             Filter: Off" << std::endl
    << "# Input B: Auto, 1M., AC, X1, Pos             Common: On" << std::endl
    << "# Ext.arm: Off                                Ref.osc: Internal" << std::endl
    << "# Hold off: Off                               Statistics: Off"  << std::endl;
  src.OccupySource();
  auto hd = src.GetHeader();
  BOOST_CHECK(hd.created_by.name     == "TimeView32");
  BOOST_CHECK(hd.created_by.version  == "1.01");
  BOOST_CHECK(hd.type_of_measurement == "FREqUENCY A");
  BOOST_CHECK(hd.time_of_start       == "MON May 12 13:13:23 2003");
  BOOST_CHECK(hd.measuring_time      == "10 ms");
  BOOST_CHECK(not hd.single);
  BOOST_CHECK(hd.input_a == "Auto, 1M., AC, X1, Pos");
  BOOST_CHECK(not hd.filter);
  BOOST_CHECK(hd.input_b == "Auto, 1M., AC, X1, Pos");
  BOOST_CHECK(hd.common);
  BOOST_CHECK(not hd.ext_arm);
  BOOST_CHECK(hd.ref_osc == "Internal");
  BOOST_CHECK(not hd.hold_off);
  BOOST_CHECK(not hd.statistics);
}

BOOST_AUTO_TEST_CASE(VoidDataSourceReadRecordTest) {
  TestSource src;
  src.data
    << "0.0000000000000e+000 9.8243659989561e+003" << std::endl
    << "3.2334289000000e-001 1.0000635974181e+007" << std::endl;
  VoidDataSource::Record rec;
  BOOST_CHECK(src.GetRecord(&rec));
  BOOST_CHECK(rec.time - 0.0000000000000e+000 < 0.0000000000001e+000);
  BOOST_CHECK(rec.value - 9.8243659989561e+003 < 0.0000000000001e+003);
  BOOST_CHECK(src.GetRecord(&rec));
  BOOST_CHECK(rec.time - 3.2334289000000e-001 < 0.0000000000001e-001);
  BOOST_CHECK(rec.value - 1.0000635974181e+007 < 0.0000000000001e+007);
  BOOST_CHECK(not src.GetRecord(&rec));
}

BOOST_AUTO_TEST_CASE(VoidDataSourceReadRecordFailTest) {
  TestSource src;
  src.data
    << "0.0000000000000e+000 9.8243659989561e+003" << std::endl
    << "3.233428900a000e-001 1.0000635974181e+007" << std::endl;
  VoidDataSource::Record rec;
  // first row
  BOOST_CHECK(src.GetRecord(&rec));
  BOOST_CHECK(rec.time - 0.0000000000000e+000 < 0.0000000000001e+000);
  BOOST_CHECK(rec.value - 9.8243659989561e+003 < 0.0000000000001e+003);
  // second row. invalid time
  BOOST_CHECK(not src.GetRecord(&rec));
  // no third row
  BOOST_CHECK(not src.GetRecord(&rec));
}

BOOST_AUTO_TEST_SUITE_END()