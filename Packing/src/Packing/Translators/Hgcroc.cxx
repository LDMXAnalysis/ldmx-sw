
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

  // construct header from buffer (in 8-bit words)
  uint32_t header = buffer.at(0) + buffer.at(1) << 8 + buffer.at(2) << 16 + buffer.at(3) << 24;

  // fill map of **electronic** IDs to the digis that were read out
  std::map<uint32_t, std::vector<ldmx::HgcrocDigiCollection::Sample>> digis;

  // look through zero-suppresion map to get the channels that have been read out
  //  right now the index of a channel is equal to its channel ID, but that won't
  //  be the case generally
  for (unsigned int i_chan{0}; i_chan < size_of_zero_supp_map; i_chan++) {
    digis[i_chan];
  }

  // loop through rest of buffer adding digis in parallel to the list of digis
  auto current_channel{digis.begin()};
  for (unsigned int i_data{1}; i_data <= buffer.size()/4-1; i_data++) {
    uint32_t data = buffer.at(4*i_data)
                  + buffer.at(4*i_data+1) << 8
                  + buffer.at(4*i_data+2) << 16
                  + buffer.at(4*i_data+3) << 24;
    int channel{-1};
    digis[i_chan].emplace_back(data);
    if (next channel) {
      current_channel++;
    }
  }
  
  ldmx::HgcrocDigiCollection unpacked_data;

  // Electronic ID <-> Detector ID while copying into collection data structure
  for (auto const& [channel, digi] : digis) {
    // TODO: Instert EID --> DetID mapping here
    unpacked_data.addDigi(channel, digi);
  }

  event.add("EcalDigis", unpacked_data);
}

}  // namespace translators
}  // namespace packing

DECLARE_PACKING_TRANSLATOR(packing::translators, Hgcroc)
