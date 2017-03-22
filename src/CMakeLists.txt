add_executable(orolia_demo
  demo.cpp
)

add_subdirectory("collector")

target_link_libraries(orolia_demo
  ${GTKMM_LIBRARIES}
  boost_program_options
  collector
)

install_targets(/ orolia_demo)