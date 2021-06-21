#ifndef PACKING_TRANSLATOR_H
#define PACKING_TRANSLATOR_H

#include <memory>
#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"

namespace packing {

/// Forward declaration for generic building function
class Translator;

/// The type for a pointer to a translator
typedef std::unique_ptr<Translator> TranslatorPtr;

/// Type of generic building function
typedef TranslatorPtr TranslatorBuilder(
    const framework::config::Parameters& ps);

/**
 * @class Translator
 *
 * The base class for all translators
 * that handle the encoding and decoding
 * of raw data that is chip-specific.
 */
class Translator {
 public:
  /**
   * Constructor used by builder function.
   */
  Translator(const framework::config::Parameters& ps) {}

  /// virtual destructor so we call derived desctructors
  virtual ~Translator() {}

  /**
   * Method used to register a translator with the factory.
   *
   * @param class_name name of class (including namespace)
   * @param builder pointer to function that can create the translator
   */
  static void declare(const std::string& class_name,
                      TranslatorBuilder* builder);

  /**
   * The Translator is given the event bus and is expected to
   * translate _everything_ that it can from the raw/encoded objects
   * to the digi/decoded objects.
   */
  virtual void decode(framework::Event& event) = 0;

  /**
   * The translator is given the event bus and is expected to
   * translate _everything_ that it can from the digi/decoded objects
   * into the raw/encoded objects.
   */
  virtual void encode(framework::Event& event) = 0;
};  // Translator

}  // namespace packing

/**
 * @macro DECLARE_PACKING_TRANSLATOR
 *
 * Defines and builder for the declared class
 * and then registers the class with the TranslatorFactory
 * as a Translator.
 */
#define DECLARE_PACKING_TRANSLATOR(NS, CLASS)                            \
  packing::TranslatorPtr CLASS##Builder(                                 \
      const framework::config::Parameters& ps) {                         \
    return std::make_unique<NS::CLASS>(ps);                              \
  }                                                                      \
  __attribute((constructor(205))) static void CLASS##Declare() {         \
    packing::Translator::declare(                                        \
        std::string(#NS) + "::" + std::string(#CLASS), &CLASS##Builder); \
  }

#endif  // PACKING_TRANSLATOR_H

