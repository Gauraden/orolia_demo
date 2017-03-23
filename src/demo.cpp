#include <string>
#include <iostream>
#include <memory>
#include <boost/program_options.hpp>
#include "demo_gui.hpp"

static
bool ParseProgramArguments(int arg_amount, char **arg_values) {
  namespace po = boost::program_options;
  po::options_description desc("Demo program for OROLIA");
	desc.add_options()
		("help", "this description")
    ("in",   po::value<std::string>()->default_value(""),
             "path to file with source data");
	po::variables_map vm;
	po::store(po::parse_command_line(arg_amount, arg_values, desc), vm);
	po::notify(vm);
	if (vm.count("help")) {
	  std::cout << desc << std::endl;
	  return false;
	}

  return true;
}

int main(int arg_amount, char **arg_values) {
  if (not ParseProgramArguments(arg_amount, arg_values)) {
    return 0;
  }
  //CreateWindow(arg_amount, arg_values);
  return 0;
}