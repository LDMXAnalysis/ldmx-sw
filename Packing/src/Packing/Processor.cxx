
#include "Packing/Processor.h"

namespace packing {

/// need to define static variable so we can have a defined symbol for it (dynamic linking nonsense)
std::map<std::string, TranslatorBuilder*> Processor::registeredTranslators_;

void Processor::registerTranslator(const std::string& class_name,
                                   TranslatorBuilder* builder) {
  if (registeredTranslators_.find(class_name) != registeredTranslators_.end()) {
    EXCEPTION_RAISE("RegistrationFail", "Translator '" + class_name +
                                            "' has already been registered.");
  }

  registeredTranslators_[class_name] = builder;
}

void Processor::configure(framework::config::Parameters& ps) {
  for (const auto& translator_params :
       ps.getParameter<std::vector<framework::config::Parameters>>(
           "translators")) {
    const auto& class_name{
        translator_params.getParameter<std::string>("class_name")};
    const auto& it{registeredTranslators_.find(class_name)};

    if (it == registeredTranslators_.end()) {
      EXCEPTION_RAISE("CreateFail", "Translator '" + class_name +
                                        "' not registered as a translator.");
    }
    translators_.emplace_back(it->second(translator_params));
  }
}

std::optional<TranslatorPtr> Processor::getTranslator(const std::string& name) const {
  if (translatorCache_.find(name) == translatorCache_.end()) {
    auto t_it{translators_.begin()};
    for (;t_it != translators_.end(); ++t_it) {
      if ((*t_it)->canTranslate(name))
        break;
    }  // loop over possible translators
  
    if (t_it == translators_.end()) {
      // unable to find translator
      return std::nullopt;
    } else {
      translatorCache_.emplace(name, *t_it);
    }
  }
  return std::optional<TranslatorPtr>{translatorCache_.at(name)};
}

}  // namespace packing

// We _do not_ DELCARE_PRODUCER because this producer is not supposed to be used directly
//    we only want to define shared functionalities for Unpacker/Packer
