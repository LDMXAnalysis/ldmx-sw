#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <iomanip>

#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <TFile.h>

#include "HGCROCv2RawData.h"
#include "RawEventFile.h"

int main(int argc, char** argv) {

  std::string m_input, m_output;
  bool m_debug = false;
  try {
    /** Define and parse the program options
     */
    namespace po = boost::program_options;
    po::options_description generic_options("Generic options");
    generic_options.add_options()("help,h", "Print help messages")
      ("input,i", po::value<std::string>(&m_input), "input file name")
      ("output,o", po::value<std::string>(&m_output)->default_value("reformatted.root"), "output file name")
      ("debug", po::bool_switch(&m_debug)->default_value(false), "Dump information to terminal to help see behavior.");

    po::options_description cmdline_options;
    cmdline_options.add(generic_options);

    po::variables_map vm;
    try {
      po::store(po::parse_command_line(argc, argv, cmdline_options), vm);
      if (vm.count("help")) {
        std::cout << generic_options << std::endl;
        return 0;
      }
      po::notify(vm);
    } catch (po::error& e) {
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
      std::cerr << generic_options << std::endl;
      return 1;
    }

    if (m_debug) {
      std::cout << "In DEBUG mode..." << std::endl;
      std::cout << "input = " << m_input << std::endl;
      std::cout << "output = " << m_output << std::endl;
    }
  } catch (std::exception& e) {
    std::cerr << "Unhandled Exception reached the top of main: " << e.what()
              << ", application will now exit" << std::endl;
    return 2;
  }

  hexareformat::HGCROCv2RawData inroc0;
  std::ifstream infile{m_input.c_str()};
  boost::archive::binary_iarchive ia{infile};

  hexareformat::RawEventFile out_file(m_output, m_debug);
  while (true) {
    try {
      ia >> inroc0;
      out_file.fill(inroc0);
    } catch (boost::archive::archive_exception&) {
      // nothing more from archive to read
      if (m_debug)
        std::cout << "Received End-Of-File from Boost archive." << std::endl;
      break;
    } catch (std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      return 1;
    }
  }

  return 0;
}
