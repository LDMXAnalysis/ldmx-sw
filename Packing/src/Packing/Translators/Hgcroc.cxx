
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

/** Old Decoding Code - will be helpful for Hgcroc Translator
  chip = rocdata.chip();
  for (half = 0; half < 2; half++) {
    // header = 0xaaBXCWADD101 : BXC 12 bit bunch crossing counter; WADD : 9 bit
    // column address of the triggered event;
    std::vector<uint32_t> data = rocdata.data(half);
    uint32_t header = data[0];
    uint32_t head = (header >> 24) & 0xff;
    corruption = head == 0xAA || head == 0x9A ? 0 : 1;
    bxcounter = (header >> 12) & 0xfff;
    wadd = (header & 0xfff) >> 3;
    for (int ichan = 0; ichan < N_READOUT_CHANNELS; ichan++) {
      if (ichan == 18) {
        channel = 37;
        adc = (data[ichan + 1] >> 10) & 0x3ff;
        tot = 0;
        toa = 0;
        outtree->Fill();
        channel = 38;
        adc = data[ichan + 1] & 0x3ff;
        tot = 0;
        toa = 0;
        outtree->Fill();
        continue;
      }
      if (ichan < 18)
        channel = ichan;
      else if (ichan > 19)
        channel = ichan - 2;
      else if (ichan == 19)
        channel = 36;
      adc = data[ichan + 1] & 0x3ff;
      toa = (data[ichan + 1] >> 10) & 0x3ff;
      tot = (data[ichan + 1] >> 20) & 0xfff;
      outtree->Fill();
    }
  }
 */
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
