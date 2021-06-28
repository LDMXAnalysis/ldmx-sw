#ifndef PACKING_PROCESSOR_H
#define PACKING_PROCESSOR_H

#include <map>
#include <vector>

#include "Framework/EventProcessor.h"
#include "Packing/Translator.h"

/**
 * @namespace packing
 *
 * Here we have our encoding/decoding routines
 * for chip-specific detector readout.
 */
namespace packing {

/**
 * @class Processor
 *
 * This producer is the base class for the unpacker and packer.
 * It handles the translators.
 */
class Processor : public framework::Producer {
 public:
  /**
   * Register a translator as available to build.
   *
   * @param class_name Full name of class (including namespace)
   * @param builder Pointer to function that can build the translator
   */
  static void registerTranslator(const std::string& class_name,
                                 TranslatorBuilder* builder);

 public:
  /// normal constructor for event producers
  Processor(const std::string& name, framework::Process& p)
      : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~Processor() {}

  /**
   * Configure the packing processor.
   *
   * This is where we decide if we are going to be trying to unpack (decode)
   * or pack (encode) and where we create the chip-specific translators
   * which will do the translation in either direction.
   *
   * @param[in] ps Parameters to configure
   */
  virtual void configure(framework::config::Parameters& ps); 

  /**
   * Actually do the packing (encoding) or unpacking (decoding).
   *
   * @param[in,out] event Event bus with inputs and outputs
   */
  virtual void produce(framework::Event& event) = 0;

 protected:
  /**
   * Get the translator that can translate the data stream
   * of the input name.
   *
   * @throws Exception if we can't find a translator for
   * the input name.
   *
   * @param[in] name Name of data stream we want to translate.
   * @return TranslatorPtr to use to translate
   */
  const TranslatorPtr& getTranslator(const std::string& name) const;

 private:
  /// A map of all registered translators
  static std::map<std::string, TranslatorBuilder*> registeredTranslators_;

  /// Cache of data stream names to translator ptrs
  mutable std::map<std::string, const TranslatorPtr&> translatorCache_;
 protected:
  /// List of translators to be translating on this run
  std::vector<TranslatorPtr> translators_;

};  // Processor

}  // namespace packing

#endif  // PACKING_PROCESSOR_H
