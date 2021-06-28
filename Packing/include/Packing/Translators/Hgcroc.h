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
  Hgcroc(const framework::config::Parameters& ps);
  virtual ~Hgcroc() {}

  bool canTranslate(const std::string& name) const final override;
  void decode(framework::Event& event, const BufferType& buffer) final override;
  BufferType encode(const ldmx::HgcrocDigiCollection& data) {
    EXCEPTION_RAISE("NoImp",
        "Hgcroc Translator hasn't implemented encode yet.");
    return {};
  }
};

}  // namespace translators
}  // namespace packing

#endif  // PACKING_TRANSLATORS_HGCROC_H
