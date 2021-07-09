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
   * Open up ROOT file and connect branches to translators and buffers.
   */
  void onProcessStart() final override;

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
  std::string raw_tree_;
  /// Should we skip branches we can't find translators for?
  bool skip_unavailable_;

 private:
  /**
   * @class SingleUnpacker
   *
   * This class handles unpacking a single buffer.
   *
   * The type of buffer used for importing the raw
   * data is defined in Packing/Translator.h
   */
  class SingleUnpacker {
   public:
    /**
     * Create a single un-packer.
     *
     * We make a new buffer object and set its address
     * as the address that the passed branch should be connected to.
     *
     * We store the translator pointer for use later.
     */
    SingleUnpacker(TBranch* br, TranslatorPtr t);

    /**
     * Clean Up the dynamically created buffer from earlier.
     *
     * We need to delete the tree we are reading from before
     * deleting this object so ROOT doesn't seg fault like a chump.
     */
    ~SingleUnpacker() { delete buffer_; }

    /**
     * Have this single unpacker decode the current buffer
     * into the passed event bus.
     *
     * We simply pass the event bus and our buffer to our translator.
     *
     * @param[out] e Event we pass to translator
     */
    void decode(framework::Event& e);
   private:
    /// Translator that can translate the branch our buffer is connected to
    TranslatorPtr translator_;
    /// Buffer we have connected to a specific branch
    BufferType* buffer_{nullptr};
  };  // SingleUnpacker

 private:
  /// ROOT file with raw data in it
  TFile *file_;
  /// ROOT tree for raw data
  TTree *tree_;
  /// current entry index in raw data tree
  long int i_entry_;
  /// Object holding the buffers and their associated translators.
  std::vector<SingleUnpacker> unpackers_;
};  // Unpacker

}  // namespace packing

#endif  // PACKING_UNPACKER_H
