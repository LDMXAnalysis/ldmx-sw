#ifndef PACKING_TRANSLATOR_H
#define PACKING_TRANSLATOR_H

#include <memory>
#include <string>

#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"

namespace packing {

/// The type of buffer we are using to hold the raw data
typedef std::vector<unsigned char> BufferType;

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
   * The translator is able to translate (encode or decode)
   * the input object by its name.
   *
   * @param[in] name A name of raw or digi object
   * @return true if this translator can translate it
   */
  virtual bool canTranslate(const std::string& name) = 0;

  /**
   * The Translator is given the event bus to put its decoded data into.
   *
   * @param[out] event Event bus to output decoded data
   * @param[in] buffer Data buffer to decode
   */
  virtual void decode(framework::Event& event, const BufferType& buffer) = 0;

  /**
   * The translator is given the data it needs to encode.
   *
   * @note This is not a pure virtual function! In order
   * to allow all translators to compile against all (potential)
   * digi types, we have the default implementation return
   * an empty buffer.
   *
   * Translator implementations should specialize this template
   * function for the DigiTypes that it can handle.
   *
   * @tparam DigiType type of data that is being encoded
   * @param data Datat that is being encoded
   * @return buffer with encoded data
   */
  template <typename DigiType>
  virtual BufferType encode(const DigiType& data) {
    return {};
  }
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

