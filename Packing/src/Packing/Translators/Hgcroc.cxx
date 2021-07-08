
#include "Packing/Translators/Hgcroc.h"

namespace packing {
namespace translators {

Hgcroc::Hgcroc(const framework::config::Parameters& ps) : Translator(ps) {}

bool Hgcroc::canTranslate(const std::string& name) const {
  static const std::string sub_str_match{"PrecisionHgcroc"};
  return (name.find(sub_str_match) != std::string::npos);
}

void Hgcroc::decode(framework::Event& event, const BufferType& buffer) {
  /** Re-sort the data from grouped by bunch to by channel
   * The readout chip streams the data off of it, so it doesn't
   * have time to re-group the signals across multiple bunches (samples)
   * by their channel ID. We need to do that here.
   */
  // fill map of **electronic** IDs to the digis that were read out
  std::map<uint32_t, std::vector<ldmx::HgcrocDigiCollection::Sample>> digis;
  auto word{buffer.begin()};
  while (word != buffer.end()) {
    try {
      // decodeBunch moves the iterator,
      // we just are here making sure to catch any out_of_range
      // exceptions due to poorly-formatted raw data
      word = decodeBunch(word, digis);
    } catch (std::out_of_range&) {
      EXCEPTION_RAISE("MisFormat",
          "Recieved raw data that was not formatted correctly.");
    }
  }

  /** Translation
   * The actual translation done here is the translation from electronic IDs
   * to detector IDs. The unpacking of the 32-bit word samples for each channel
   * is done by the HgcrocDigiCollection::Sample class and is done on-the-fly
   * in order to save disk-space.
   */
  ldmx::HgcrocDigiCollection unpacked_data;
  // Electronic ID <-> Detector ID while copying into collection data structure
  for (auto const& [eid, digi] : digis) {
    // TODO: Instert EID --> DetID mapping here
    auto channel = eid;
    unpacked_data.addDigi(channel, digi);
  }

  event.add("EcalDigis", unpacked_data);
}

std::vector<uint64_t>::iterator Hgcroc::decodeBunch(
    std::vector<uint64_t>::iterator start,
    std::map<uint64_t, std::vector<ldmx::HgcrocDigiCollection::Sample>> data) {
  /** Decode Bunch Header
   * We have a few words of header material before the actual data.
   * This header material is assumed to be encoded as in Table 3
   * of the DAQ specs.
   *
   * <name> (bits)
   *
   * VERSION (4) | FPGA_ID (8) | NLINKS (6) | 0 | LEN (12)
   * BX ID (12) | RREQ (10) | OR (10)
   * RID ok (1) | CDC ok (1) | LEN3 (6) |
   *  RID ok (1) | CDC ok (1) | LEN2 (6) |
   *  RID ok (1) | CDC ok (1) | LEN1 (6) |
   *  RID ok (1) | CDC ok (1) | LEN0 (6)
   * ... other listing of links ...
   */
  auto word{start};
  unsigned int version{(*word >> 12 + 1 + 6 + 8) & mask<4>::m};
  if (version != 1)
    EXCEPTION_RAISE("VersMis", "Hgcroc Translator only knows version 1.");

  unsigned int fpga{(word >> 12 + 1 + 6) & mask<8>::m};
  unsigned int nlinks{(word >> 12 + 1) & mask<6>::m};
  unsigned int len{word & mask<12>::m};

  word++;

  unsigned int bx_id{(word >> 10 + 10) & mask<12>::m};
  unsigned int rreq{(word >> 10) & mask<10>::m};
  unsigned int orbit{word & mask<10>::m};

  std::vector<unsigned int> num_channels_per_link;
  for (unsigned int i_link{0}; i_link < nlinks; i_link++) {
    if (i_link % 4 == 0) word++;
    unsigned int shift_in_word{8 * i_link % 4};
    bool rid_ok{(*word >> shift_in_word + 7) & mask<1>::m == 1};
    bool cdc_ok{(*word >> shift_in_word + 6) & mask<1>::m == 1};
    num_channels_per_link[i_link] = (*word >> shift_in_word) & mask<6>::m;
  }

  /** Decode Each Link in Sequence
   * Now we should be decoding each link serially
   * where each link was encoded as in Table 4 of
   * the DAQ specs
   *
   * ROC_ID (16) | CRC ok (1) | 00000 | RO Map (8)
   * RO Map (32)
   */

  for (unsigned int i_link{0}; i_link < nlinks; i_link++) {
    // move on from last word counting links or previous link
    word++;
    unsigned int roc_id{(*word >> 8 + 5 + 1) & mask<16>::m};
    bool crc_ok{(*word >> 8 + 5) & mask<1>::m == 1};
    unsigned int ro_map_39_32{*word & mask<8>::m};
    word++;
    unsigned int ro_map_31_0{*word & mask<32>::m};

    // loop through channels on this link, 
    //  check if they have been readout before saving word
    for (unsigned int i_chan{0}; i_chan < 40; i_chan++) {
      bool has_been_read{false};
      if (i_chan < 32) {
        has_been_read = ((ro_map_31_0 >> i_chan & mask<1>::m) == 1)
      } else {
        has_been_read = ((ro_map_39_32 >> (i_chan - 31) & mask<1>::m) == 1)
      }

      // skip zero-suppressed channels
      if (not has_been_read) continue;

      // next word is this channel
      word++;

      if (i_chan == 0) { 
        /** Special "Header" Word from ROC
         * 0101 | BXID (12) | RREQ (6) | OR (3) | HE (3) | 0101
         */
      } else if (i_chan == 1) {
        /** Common Mode Channels
         * 10 | 0000000000 | Common Mode ADC 0 (10) | Common Mode ADC 1 (10)
         */
      } else if (i_chan == 39) {
        /** CRC Checksum form ROC
         */
        unsigned int roc_crc{*word & mask<32>::m};
      } else {
        /// DAQ Channels
  
        /** Generate Electronics ID
         * Link Index i_link
         * Channel Index i_chan
         * ROC ID roc_id
         * FPGA ID fpga
         * are all available.
         * For now, we just generate a dummy mapping
         * using the link and channel indices.
         */
        unsigned int eid{i_link * 100 + i_chan};
  
        // copy data into EID->sample map
        data[eid].emplace_back(*word);
      }  // type of channel
    }    // loop over channels (j in Table 4)
  }      // loop over links

  // next word is CDC checksum
  word++;
  unsigned int cdc{*word & mask<32>::m};

  // need to go one word past the last word we used.
  word++;
  return word;
}

}  // namespace translators
}  // namespace packing

DECLARE_PACKING_TRANSLATOR(packing::translators, Hgcroc)
