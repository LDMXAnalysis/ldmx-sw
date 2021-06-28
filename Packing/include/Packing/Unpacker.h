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

 private:
  /// name of raw data to unpack
  std::string raw_name_;

  /// pass of raw data to unpack
  std::string raw_pass_;
};  // Unpacker

}  // namespace packing

#endif  // PACKING_UNPACKER_H
