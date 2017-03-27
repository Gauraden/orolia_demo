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
             "path to file with source data")
    ("bsize", po::value<unsigned>()->default_value(800),
             "size of buffer, for storing loaded records");
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
    std::cout << "Settings: \n"
              << " * file  : " << vm["in"].as<std::string>() << ";\n"
              << " * buffer: " << vm["bsize"].as<unsigned>() << " records;\n";
    out->UseCompressor(new Compressor(vm["bsize"].as<unsigned>()));
    out->UseDataSource(new FileDataSource(vm["in"].as<std::string>()));
  } catch (...) {
    return false;
  }
  return true;
}

static
void PrintCollectorMessages(const Collector::Messages &msgs) {
  std::for_each(msgs.begin(), msgs.end(), [](const std::string &msg) {
    std::cout << "\t - " << msg << std::endl;
  });
}

int main(int arg_amount, char **arg_values) {
  Collector   cl;
  GuiSettings gui_opts;
  if (not ParseProgramArguments(arg_amount, arg_values, &cl)) {
    return 0;
  }
  std::cout << "Loading records ..." << std::endl;
  if (not cl.Begin() || not cl.FetchAllRecords()) {
    std::cout << "Failed to read records: " << std::endl;
    PrintCollectorMessages(cl.GetMessages());
    cl.End();
    return 1;
  }
  cl.End();
  std::cout << "Drawing graph ... " << std::endl;
  PrintCollectorMessages(cl.GetMessages());
  CreateWindowWithChart(cl.GetCompressor(), gui_opts);
  return 0;
}