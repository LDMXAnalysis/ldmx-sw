#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <iomanip>

#include <boost/cstdint.hpp>
#include <boost/program_options.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <yaml-cpp/yaml.h>

#include <TFile.h>

#include "HGCROCv2RawData.h"
#include "LinkAligner.h"
#include "datawriter.h"

int main(int argc, char** argv)
{

  std::string m_input,m_output,m_outputType;
  std::string m_metaDataYaml;
  bool m_printArgs = false;
  try { 
    /** Define and parse the program options 
     */ 
    namespace po = boost::program_options; 
    po::options_description generic_options("Generic options"); 
    generic_options.add_options()
      ("help,h", "Print help messages")
      ("input,i",        po::value<std::string>(&m_input), "input file name")
      ("output,o",       po::value<std::string>(&m_output)->default_value("toto.root"), "output file name")
      ("metaDataYaml,M", po::value<std::string>(&m_metaDataYaml), "yaml (created at the same time as the aw data file) file containing meta data of the run")
      ("outputType,t",   po::value<std::string>(&m_outputType)->default_value("root"), "data type option -> will decide what kind of unpacker is used and what kind of ntuple is saved; available choice : root, unpacked, delayscan")
      ("printArgs", po::bool_switch(&m_printArgs)->default_value(false), "turn me on to print used arguments");

    po::options_description cmdline_options;
    cmdline_options.add(generic_options);
    
    po::variables_map vm; 
    try { 
      po::store(po::parse_command_line(argc, argv, cmdline_options),  vm); 
      if ( vm.count("help")  ) { 
        std::cout << generic_options   << std::endl; 
        return 0; 
      } 
      po::notify(vm);
    }
    catch(po::error& e) { 
      std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
      std::cerr << generic_options << std::endl; 
      return 1; 
    }
    
    if( m_printArgs ){
	std::cout << "input = "        << m_input        << std::endl;
	std::cout << "output = "       << m_output       << std::endl;
	std::cout << "metaDataYaml = " << m_metaDataYaml << std::endl;
	std::cout << "outputType = "   << m_outputType   << std::endl;	
	std::cout << std::endl;
      }

    if( vm.count("outputType") && (! (m_outputType == "root" || 
				      m_outputType == "unpacked" ||
				      m_outputType == "delayscan" )) ){
      std::cout << "Invalid choice for option 'outputType'. Allowed choices : 'root', 'unpacked'" << std::endl;
      throw po::validation_error(po::validation_error::invalid_option_value, "outputType");
    }
  }
  catch(std::exception& e) { 
    std::cerr << "Unhandled Exception reached the top of main: " 
              << e.what() << ", application will now exit" << std::endl; 
    return 2; 
  }   

  HGCROCv2RawData inroc0;
  link_aligner_data lad;
  std::ifstream infile{ m_input.c_str() };
  boost::archive::binary_iarchive ia{infile};

  DataWriterFactory fac;
  std::unique_ptr<Writer> writer;

  if( m_metaDataYaml.empty() ){
    if( m_outputType=="root" ){
      writer = fac.Create("root",m_output);
      std::cout << "save root data in : " << *writer << std::endl;
    }
    else if( m_outputType=="unpacked" ){
      writer = fac.Create("unpacked",m_output);
      std::cout << "save run summary root data in : " << *writer << std::endl;
    }
    else if( m_outputType=="delayscan" ){
      writer = fac.Create("delayscan",m_output);
      std::cout << "save delayscan root data in : " << *writer << std::endl;
    }
    else{
      std::cout << "ERROR which should not happen" << std::endl;
      return 0;
    }
  }
  else{
    YAML::Node config = YAML::LoadFile(m_metaDataYaml.c_str())["metaData"];
    writer = fac.Create("yamlnode",m_output,config);
    std::cout << "save run summary root data in : " << *writer << std::endl;
  }
  
  while(1){
    try{
      if( m_outputType=="delayscan" ){
	ia >> lad;
	writer->fill(lad);
      }
      else{
	ia >> inroc0;
	writer->fill(inroc0);
      }
    }
    catch( std::exception& e ){
      // std::cout << e.what() << std::endl;
      break;
    }
  }
  writer->save();
  infile.close();

  return 0;
}
