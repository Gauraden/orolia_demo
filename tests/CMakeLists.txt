add_executable(units_tests
  units_tests.cpp
  test_data_source.cpp
)

target_link_libraries(units_tests
  boost_unit_test_framework
  collector
)

install_targets(/ units_tests)