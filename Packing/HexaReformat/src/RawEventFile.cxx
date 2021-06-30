#include <sstream>
#include <cstring>
#include "RawEventFile.h"

namespace packing {
namespace hexareformat {

RawEventFile::RawEventFile(std::string filename) {
  file_ = std::make_unique<TFile>(filename.c_str(), "RECREATE");

  // TFile cleans up the TTrees that are created within it
  data_tree_ = new TTree("LDMX_RawData","Encoded raw data for LDMX");
  data_tree_->Branch("data", &raw_data_);
  data_tree_->Branch("event", &event_);
  data_tree_->Branch("run", &run_);

  // TFile cleans up the TTrees that are created within it
  runs_tree_ = new TTree("LDMX_Runs","Run for this raw data.");
  runs_tree_->Branch("run", &run_);
}

RawEventFile::~RawEventFile() {
  endRun();
  file_->Write();
  file_->Close();
}

void RawEventFile::fill(HGCROCv2RawData rocdata) {
  if (rocdata.event() != event_) {
    if (event_ > 0) endEvent();
    event_ = rocdata.event()
  }

  for (int half{0}; half < 2; half++) {
    const std::vector<uint32_t>& sample_from_each_channel{rocdata.data(half)};
    uint32_t header{sample_from_each_channel.at(0)};
    for (int i_chan{0}; i_chan < N_READOUT_CHANNELS; i_chan++) {
      // deduce "electronic ID" for this ROC and get the raw sample
      unsigned int eid{i_chan+half*100};
      intermediate_buffer_[eid].push_back(sample_from_each_channel.at(i_chan+1));
    }
  }
}

void RawEventFile::endEvent() {
  raw_data_.clear();
  /**
   * Insert new-event header here
   */
  auto& buffer{raw_data_["EcalPrecisionReadout"]};
  buffer.push_back(0xFFFF);

  /**
   * Translate to our expected buffer
   */
  uint64_t zero_supp_map{0x0000};
  unsigned int upper_lim_eid{200};
  for (unsigned int eid{0}; eid <= upper_lim_eid; eid++) {
    if (intermediate_buffer_.find(eid) != intermediate_buffer_.end()) {
      // add 1 to zero-suppression map
      zero_supp_map += 1 << (upper_lim_eid-eid)
    }
  }
  buffer.push_back(zero_supp_map);

  for (const auto&[eid, data] : intermediate_buffer_) {
    buffer.append(data.begin(), data.end());
  }

  /**
   * Fill data tree
   */
  data_tree_->Fill();

  /**
   * Reset intermediate data mapping
   */
  intermediate_map_.clear();
}

}  // hexareformat
}  // packing

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
