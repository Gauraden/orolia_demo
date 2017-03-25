#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>
#include "../src/collector/compressor.hpp"

struct CompressorTestFixture {
    CompressorTestFixture() {}
    ~CompressorTestFixture() {}
};

static
bool CheckRec(
    const Compressor::Record &rec,
    double                    tm_f,
    double                    tm_s,
    double                    val_f,
    double                    val_s) {
  return (
    std::abs(rec.time.first   - tm_f ) < 0.1 &&
    std::abs(rec.time.second  - tm_s ) < 0.1 &&
    std::abs(rec.value.first  - val_f) < 0.1 &&
    std::abs(rec.value.second - val_s) < 0.1
  );  
}

static
bool CheckRec(
    const Compressor::Record &f_rec,
    const Compressor::Record &s_rec) {
  return (
    std::abs(f_rec.time.first   - s_rec.time.first  ) < 0.1 &&
    std::abs(f_rec.time.second  - s_rec.time.second ) < 0.1 &&
    std::abs(f_rec.value.first  - s_rec.value.first ) < 0.1 &&
    std::abs(f_rec.value.second - s_rec.value.second) < 0.1
  );
}
// -----------------------------------------------------------------------------
// Инициализация набора тестов
BOOST_FIXTURE_TEST_SUITE(CompressorTestSuite, CompressorTestFixture)

BOOST_AUTO_TEST_CASE(CompressorRecordMergeTest) {
  typedef Compressor::Record Rec;
  Rec rec_0_1__0_1(1, 1);
  Rec rec_2_3__2_3(3, 3);
  Rec rec_4_5__3_4(5, 3);
  // invalid order of time point
  BOOST_CHECK(not rec_0_1__0_1.MergeWith(Rec(2, 1)));
  // time : 0.0 - 1.0
  // value: 0.0 - 1.0
  BOOST_CHECK(rec_0_1__0_1.MergeWith(Rec(0, 0)));
  BOOST_CHECK(CheckRec(rec_0_1__0_1, 0.0, 1.0, 0.0, 1.0));
  // time : 2.0 - 3.0
  // value: 2.0 - 3.0
  BOOST_CHECK(rec_2_3__2_3.MergeWith(Rec(2, 2)));
  BOOST_CHECK(CheckRec(rec_2_3__2_3, 2.0, 3.0, 2.0, 3.0));
  // time : 4.0 - 5.0
  // value: 3.0 - 4.0 
  BOOST_CHECK(rec_4_5__3_4.MergeWith(Rec(4, 4)));
  BOOST_CHECK(CheckRec(rec_4_5__3_4, 4.0, 5.0, 3.0, 4.0));
  // invalid order of time point
  BOOST_CHECK(not rec_0_1__0_1.MergeWith(rec_2_3__2_3));
  // time : 0.0 - 3.0
  // value: 0.0 - 3.0
  BOOST_CHECK(rec_2_3__2_3.MergeWith(rec_0_1__0_1));
  // time : 0.0 - 5.0
  // value: 0.0 - 4.0
  BOOST_CHECK(rec_4_5__3_4.MergeWith(rec_2_3__2_3));
  
  Rec rec_0_1__1_10(1, 10);
  Rec rec_2_3__2_11(3, 11);
  // time : 0.0 - 1.0
  // value: 1.0 - 10.0
  BOOST_CHECK(rec_0_1__1_10.MergeWith(Rec(0, 1)));
  // time : 2.0 - 3.0
  // value: 2.0 - 11.0
  BOOST_CHECK(rec_2_3__2_11.MergeWith(Rec(2, 2)));
  // time : 0.0 - 3.0
  // value: 1.0 - 11.0
  BOOST_CHECK(rec_2_3__2_11.MergeWith(rec_0_1__1_10));
  BOOST_CHECK(CheckRec(rec_2_3__2_11, 0.0, 3.0, 1.0, 11.0));
}

BOOST_AUTO_TEST_CASE(CompressorPushTest) {
  const size_t kSrcAmount = 10;
  const size_t kResAmount = 5;
  VoidDataSource::Record src_recs[kSrcAmount] = {
    {0, 1},
    {1, 10},
    {2, 2},
    {3, 11},
    {4, 3},
    {5, 12},
    {6, 4},
    {7, 13},
    {8, 5},
    {9, 14}
  };
  Compressor::Record res_recs[kResAmount] = {
    {{0, 3 }, {1, 11}},
    {{4, 5 }, {3, 12}},
    {{6, 7 }, {4, 13}},
    {8, 5},
    {9, 14}
  };
  Compressor cr(kResAmount);
  for (size_t i = 0; i < kSrcAmount; ++i) {
    cr.PushRecord(src_recs[i]);
  }
  size_t i = 0;
  auto records = cr.GetRecords();
  auto rit     = records.begin();
  // 2 last records is not merged
  for (; i < kResAmount - 2 && rit != records.end(); ++i, ++rit) {
    BOOST_CHECK(CheckRec(*rit, res_recs[i]));
  }
  BOOST_CHECK(rit != records.end());
  BOOST_CHECK(i   == kResAmount - 2);
}

BOOST_AUTO_TEST_CASE(CompressorPrecalculateScalesTest) {
  Compressor cpr(10);
  cpr.PrecalculateScales({1, 10});
  cpr.PrecalculateScales({2, 20});
  Compressor::Range pt[2];
  auto num = cpr.CastRecordToScales({1.5, 15}, pt);
  BOOST_CHECK(num == 1);
  BOOST_CHECK(pt[0].first  == 0.5);
  BOOST_CHECK(pt[0].second == 0.5);
}

BOOST_AUTO_TEST_SUITE_END()