#include <string>
#include <iostream>
#include <memory>
#include <boost/program_options.hpp>
#include "demo_gui.hpp"
#include "collector/collector.hpp"

static
bool ParseProgramArguments(int arg_amount, char **arg_values, Collector *out) {
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
  if (not vm.count("in")) {
    std::cout << "You need to set <in> argument! Please look at <help>"
              << std::endl;
    return false;
  }
  try {
    out->UseCompressor(new Compressor(1024));
    out->UseDataSource(new FileDataSource(vm["in"].as<std::string>()));
  } catch (...) {
    return false;
  }
  return true;
}

int main(int arg_amount, char **arg_values) {
  Collector cl;
  if (not ParseProgramArguments(arg_amount, arg_values, &cl)) {
    return 0;
  }
  //CreateWindow(arg_amount, arg_values);
  if (not cl.Begin()) {
    std::cout << "Failed to init collector: " << cl.GetErrorMessage() << std::endl;
    return 1;
  }
  if (not cl.FetchAllRecords()) {
    std::cout << "Failed to fetch all records: " << cl.GetErrorMessage() << std::endl;
    return 2;
  }
  cl.End();
  return 0;
}