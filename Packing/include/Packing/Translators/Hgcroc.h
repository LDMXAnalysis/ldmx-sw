#ifndef PACKING_TRANSLATORS_HGCROC_H
#define PACKING_TRANSLATORS_HGCROC_H

#include "Recon/Event/HgcrocDigiCollection.h"

#include "Packing/Translator.h"

namespace packing {
namespace translators {

/**
 * @class Hgcroc
 * Translator for data coming out of the HGC ROC.
 */
class Hgcroc : public Translator {
 public:
  /**
   * Constructor
   *
   * Currently, we have no parameters specific for this translator.
   */
  Hgcroc(const framework::config::Parameters& ps);

  /// default destructor
  virtual ~Hgcroc() = default;

  /**
   * We can translate the "precision digis" output by the HGC ROC.
   *
   * They are called "precision" in order to make them distinct
   * from the "trigger" digis which are merged into groups for 
   * speed.
   *
   * @param[in] name Name of data stream to check
   * @return true if name has 'PrecisionHgcroc' as a sub string
   */
  bool canTranslate(const std::string& name) const final override;
  
  /**
   * Decode the input buffer into an event object and put that event
   * object onto the event bus.
   *
   * We assume that the data is in the format specified by the
   * ECal data format specifications.
   *
   * @TODO Can we assume that the bunches are ordered correctly?
   * @TODO Electronics ID from link, channel, ROC, FPGA
   * @TODO ELectronics to Detector ID
   *
   * @param[in,out] event EventBus to put object onto
   * @param[in] buffer RawData buffer to decode
   */
  void decode(framework::Event& event, const BufferType& buffer) final override;
  
  /**
   * Encoding the C++ digi into a raw event type is delayed
   * until we more strictly define our resulting raw data format.
   */
  BufferType encode(const ldmx::HgcrocDigiCollection& data) {
    EXCEPTION_RAISE("NoImp",
        "Hgcroc Translator hasn't implemented encode yet.");
    return {};
  }
};

}  // namespace translators
}  // namespace packing

#endif  // PACKING_TRANSLATORS_HGCROC_H
