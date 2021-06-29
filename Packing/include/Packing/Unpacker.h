#ifndef PACKING_UNPACKER_H
#define PACKING_UNPACKER_H

#include "Packing/Processor.h"

namespace packing {

/**
 * @class Unpacker
 *
 * This producer unpacks (decodes) the raw data using
 * the translators handled by Processor.
 */
class Unpacker : public Processor {
 public:
  /// normal constructor
  Unpacker(const std::string& name, framework::Process& p)
    : Processor(name, p) {}
  /// empty destructor
  virtual ~Unpacker() {}

  /**
   * Configure the unpacker
   *
   * We need to call the base class configure to create
   * the necessary translators.
   *
   * @param[in] ps Parameters for configuration
   */
  void configure(framework::config::Parameters& ps) final override;

  /**
   * Actually do the unpacking/decoding.
   *
   * @param[in,out] event Event bus with raw data where we will put the digis
   */
  void produce(framework::Event& event) final override;

  /**
   * Close up ROOT file with raw data in it.
   */
  void onProcessEnd() final override;

 private:
  /// name of ROOT file with raw data in it
  std::string raw_file_;
  /// name of tree in raw file
  std::string raw_tree_{"RawData"};
  /// name of data branch in raw file
  std::string raw_name_{"data"};

 private:
  /// ROOT file with raw data in it
  TFile *file_;
  /// ROOT tree for raw data
  TTree *tree_;
  /// current entry (event)
  long int i_entry_;
  /**
   * Object holding raw data
   *
   * The type of buffer used for importing the raw
   * data is defined in Packing/Translator.h
   */
  std::map<std::string,BufferType> raw_data_;
};  // Unpacker

}  // namespace packing

#endif  // PACKING_UNPACKER_H
