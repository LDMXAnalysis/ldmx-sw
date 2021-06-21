
#include "Packing/Translator.h"

#include "Packing/Processor.h"

namespace packing {

void Translator::declare(const std::string& class_name,
                         TranslatorBuilder* builder) {
  packing::Processor::registerTranslator(class_name, builder);
}

}  // namespace packing
