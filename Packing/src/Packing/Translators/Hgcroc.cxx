
#include "Packing/Translators/Hgcroc.h"


namespace packing {
namespace translators {

Hgcroc::Hgcroc(const framework::config::Parameters& ps) : Translator(ps) {
}

bool Hgcroc::canTranslate(const std::string& name) const {
  static const std::string sub_str_match{"hgcroc"};
  return (name.find(sub_str_match) != std::string::npos);
}

void Hgcroc::decode(framework::Event& event, const BufferType& buffer) {
  ldmx::HgcrocDigiCollection unpacked_data;
  
  // do some decoding and EID mapping

  event.add("EcalDigis", unpacked_data);
}

}  // namespace translators
}  // namespace packing

DECLARE_PACKING_TRANSLATOR(packing::translators, Hgcroc)
