
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
  decode_ = ps.getParameter<bool>("decode");

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

void Processor::produce(framework::Event& event) {
  for (const auto& t : translators_) {
    if (decode_)
      t->decode(event);
    else
      t->encode(event);
  }
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Processor)
