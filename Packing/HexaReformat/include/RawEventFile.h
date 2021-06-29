#ifndef PACKING_HEXAREFORMAT_RAWEVENTFILE_H_
#define PACKING_HEXAREFORMAT_RAWEVENTFILE_H_

#include <iostream>
#include <map>
#include <TFile.h>
#include <TDirectory.h>
#include <TTree.h>

#include "Packing/HexaReformat/HGCROCv2RawData.h"

/**
 * The number of readout channels is defined by the hardware
 * construction of a HGC ROC.
 */
#define N_READOUT_CHANNELS 38

namespace packing {
namespace hexareformat {

/**
 * @class RawEventFile
 *
 * Here we are constructing the RawEventFile that can be read
 * by the Unpacker processor.
 */
class RawEventFile {
 public:
  /**
   * Constructor - open a root file for writing
   * for the input filename.
   */
  RawEventFile(std::string filename)

  /**
   * Close up the root file
   */
  ~RawEventFile();

  /**
   * Copy the input raw data into our format.
   *
   * We fill the output data tree and reset if
   * we encounter an event number that is different than our current one.
   */
  void fill(HGCROCv2RawData rocdata);

 private:
  /**
   * End of Event
   *
   * Copy the readout data into our style of buffer
   * and then fill the output data tree.
   */
  void endEvent();

  /**
   * End of Run
   *
   * Fill the run tree
   */
  void endRun();

 private:
  /// The output file we are writing to
  std::unique_ptr<TFile> file_;
  /// The tree with the data branches
  TTree* data_tree_;
  /// The tree with the run branches
  TTree* runs_tree_;

  /// The packed data
  std::map<std::string,std::vector<uint64_t>> raw_data_;
  /// Event number
  int event_{-1};
  /// Run number
  int run_;

  /// Intermediat map from Electronics ID to Raw Data buffer
  std::map<uint32_t,std::vector<uint32_t>> intermediate_buffer_;
};  // RawEventFile

}  // hexareformat
}  // packing

#endif
