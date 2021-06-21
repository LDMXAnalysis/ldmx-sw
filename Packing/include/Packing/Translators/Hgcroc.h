#ifndef PACKING_TRANSLATORS_HGCROC_H
#define PACKING_TRANSLATORS_HGCROC_H

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

  void decode(framework::Event& event) final override;
  void encode(framework::Event& event) final override;

 private:
  /// pass name regular expression
  std::string pass_regex_;

  /// object name regular expression
  std::string object_regex_;

};

}  // namespace translators
}  // namespace packing

#endif  // PACKING_TRANSLATORS_HGCROC_H
