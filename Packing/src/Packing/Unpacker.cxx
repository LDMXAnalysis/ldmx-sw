
#include "Packing/Unpacker.h"

namespace packing {

void Unpacker::configure(framework::config::Parameters& ps) {
  // create the configured translators
  Processor::configure(ps);

  raw_name_ = ps.getParameter<std::string>("raw_name");
  raw_pass_ = ps.getParameter<std::string>("raw_pass");
}

void Unpacker::produce(framework::Event& event) {
  // The type of object we use as a data buffer is defined in Packing/Translator.h
  const auto& raw_data{event.getMap<std::string,BufferType>(raw_name_,raw_pass_)};

  for (const auto&[name, buffer] : raw_data) {
    getTranslator(name)->decode(event, buffer);
  }  // loop over raw data packages
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Unpacker)
