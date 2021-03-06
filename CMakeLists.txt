cmake_minimum_required(VERSION 3.5)
project("orolia_demo")

find_package(PkgConfig REQUIRED)
pkg_check_modules (GTKMM REQUIRED gtkmm-3.0)

if (WIN32)
  set(BOOST_POSTFIX '-mt')
endif ()

add_definitions(
  -static
  --std=gnu++14
  -Wunused-parameter
  -Wnon-virtual-dtor
  -Wignored-qualifiers
  -Wmissing-declarations
  -Wsign-compare
  -Wreturn-type
)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})

add_subdirectory("src")
add_subdirectory("tests")