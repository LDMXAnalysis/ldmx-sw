#ifndef PACKING_HEXAREFORMAT_RAWEVENTFILE_H_
#define PACKING_HEXAREFORMAT_RAWEVENTFILE_H_

#include <iostream>
#include <map>
#include <TFile.h>
#include <TDirectory.h>
#include <TTree.h>

#include "HGCROCv2RawData.h"

/**
 * The number of readout channels is defined by the hardware
 * construction of a HGC ROC as well as our DAQ readout specs.
 *
 * We have defined a method for keeping track of zero suppression
 * along the DAQ path using a bit-map. Since we aren't using
 * zero suppression currently in the hexactrl-sw DAQ, this ends
 * up being a map of all ones.
 *
 * We have 38 channels that are always read out.
 */
#define N_READOUT_CHANNELS 38

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
  RawEventFile(std::string filename);

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
  std::vector<uint64_t> buffer_;
  /// Event number
  int event_{-1};
  /// Run number
  int run_{333};
};  // RawEventFile

}  // hexareformat

#endif
