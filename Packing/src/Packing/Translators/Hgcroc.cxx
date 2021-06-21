
#include "Packing/Translators/Hgcroc.h"

#include "Recon/Event/HgcrocDigiCollection.h"

namespace packing {
namespace translators {

Hgcroc::Hgcroc(const framework::config::Parameters& ps) : Translator(ps) {
  pass_regex_ = ps.getParameter<std::string>("pass_regex");
  object_regex_ = ps.getParameter<std::string>("object_regex");
}

void Hgcroc::decode(framework::Event& event) {
  static std::vector<framework::ProductTag> product_tag_cache;
  if (product_tag_cache.empty()) {
    product_tag_cache = event.searchProducts(object_regex_,"",pass_regex_);
  }

  std::cout << "Hgcroc::decode" << std::endl;
}

void Hgcroc::encode(framework::Event& event) {
  static std::vector<framework::ProductTag> product_tag_cache;
  if (product_tag_cache.empty()) {
    product_tag_cache = event.searchProducts(object_regex_,"HgcrocDigiCollection",pass_regex_);
  }

}

}  // namespace translators
}  // namespace packing

DECLARE_PACKING_TRANSLATOR(packing::translators, Hgcroc)
