#include <sstream>
#include <cstring>
#include "RawEventFile.h"

namespace hexareformat {

/**
 * Generate bit masks at compile time.
 *
 * The template input defines how many of
 * the lowest bits will be masked for.
 *
 * Use like:
 *
 *  i & mask<N>::m
 *
 * To mask for the lowest N bits in i.
 */
template <unsigned int n_bits>
struct mask {
  static const uint64_t m = (1 << n_bits) - 1;
};

RawEventFile::RawEventFile(std::string filename) {
  file_ = std::make_unique<TFile>(filename.c_str(), "RECREATE");

  // TFile cleans up the TTrees that are created within it
  data_tree_ = new TTree("LDMX_RawData","Encoded raw data for LDMX");
  data_tree_->Branch("EcalPrecisionHgcrocReadout", &buffer_);
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
  // arbitrary ID values that are meaningless right now
  static const int fpga{42};
  static const int orbit{13};
  static const int roc{7};
  // current word we are using to help create buffer
  static uint64_t word;

  if (rocdata.event() != event_) {
    if (event_ > 0) endEvent();
    event_ = rocdata.event();
  }

  /** Insert new header here
   * In the DAQ specs, the header is separated into 32-bit words,
   * so we do the same here. Below, I've copied down the structure
   * that I've implemented for ease of reference.
   * The numbers in parentheses are the number of bits.
   *
   * VERSION (4) | FPGA (8) | NLINKS(6) | 0 | TOTAL_RO(12)
   * BX ID (12) | RREQ (10) | OR (10)
   *
   * We need to do some fancy footwork to extract the BX ID from the
   * ROC data because it is not decoded in hexactrl-sw by default.
   */
  word = 
    (1 & mask<4>::m << 8+6+1+12) + // 4 bit version number
    (fpga & mask<8>::m << 6+1+12) + // 8 bit FPGA ID # (arbitrarily set to 9 here)
    (2 & mask<6>::m << 1+12) + // 6 bit number of links ("halves" of ROC)
    (0 & mask<1>::m << 1) + // Reserved 0 bit
    (2*N_READOUT_CHANNELS & mask<12>::m << 0); // 12 bit total number of readout channels
  buffer_.push_back(word);

  unsigned int bx_id{0};
  try {
    unsigned int bx_id_0{(rocdata.data(0).at(0) >> 12) & 0xfff};
    unsigned int bx_id_1{(rocdata.data(1).at(0) >> 12) & 0xfff};
    if (bx_id_0 == bx_id_1)
      bx_id = bx_id_0;
    /** Don't worry about this for now...
    else
      throw std::runtime_error("Received two different BX IDs at once.");
      */
  } catch (std::out_of_range&) {
    throw std::runtime_error("Received ROC data without a header.");
  }

  /*
  if (bx_id == 0)
    throw std::runtime_error("Unable to deduce BX ID.");
    */

  word = 
    (bx_id & mask<12>::m << 10+10) + // 12 bit bunch ID number
    (event_ & mask<10>::m << 10) + // 10 bit read request ID number (what will be an event number)
    (orbit & mask<10>::m << 0); // 10 bit bunch train/orbit counter
  buffer_.push_back(word);

  /** Insert link counters
   * We only have two links in this ROC,
   * so the first half of this word will be zeroes.
   *
   * And then, we aren't doing any zero-suppression,
   * so the number of channels readout will always
   * be the same for both links.
   * The numbers in parentheses are the number of bits.
   *
   * sixteen zeros 
   *  | RID ok (1) | CRC ok (1) | LEN1 (6) 
   *  | RID ok (1) | CRC ok (1) | LEN0 (6)
   */
  word = 
    (0 & mask<16>::m << 16) +
    (1 & mask<1>::m << 6+1+1+6+1) +
    (1 & mask<1>::m << 6+1+1+6) +
    (N_READOUT_CHANNELS & mask<6>::m << 6+1+1) +
    (1 & mask<1>::m << 6+1) +
    (1 & mask<1>::m << 6) +
    (N_READOUT_CHANNELS & mask<6>::m << 0);
  buffer_.push_back(word);

  /** Go through both of our links
   * In our case, the two "half"s of the ROC are our two links.
   *
   * The hexactrl-sw does not insert the header for each link
   * because there is no zero-suppression or other ROCs to worry about,
   * so we need to insert an additional header here.
   *
   * ROC ID (16) | CRC ok (1) | 00000 | Readout Map [39:32]
   *
   * The numbers in parentheses are the number of bits.
   */
  for (int half{0}; half < 2; half++) {
    word = 
      (roc & mask<16>::m << 8+5+1) + // 16 bits for ROC ID (arbitrary choice of 7)
      (1 & mask<1>::m << 8+5) + // CRC OK bit
      (0 & mask<5>::m << 8) + // 5 bits of zero reserved
      (mask<8>::m); // last 8 bits of readout map (everything is being read out)
    buffer_.push_back(word);
    // rest of readout map (everything is being readout)
    buffer_.push_back(0xFFFF);
    /** header word from ROC
     * 0101 | BX ID (12) | RREQ (6) | OR (3) | HE (3) | 0101
     */
    word = 
      (0b0101 << 4+3+3+6+12) + // 4 bits 0101
      (bx_id & mask<12>::m << 4+3+3+6) + //12 bits for BX ID
      (event_ & mask<6>::m << 4+3+3) + //6 bits for RREQ (event)
      (orbit & mask<3>::m << 4+3) + //lower 3 bits of orbit (bunch train)
      (0 & mask<3>::m << 4) + //any Hamming errors present?
      (0b0101); // 4 bits 0101
    buffer_.push_back(word);

    // copy in _data_ words from hexactrl-sw
    //  (we already decoded the header to get the BX ID above)
    const std::vector<uint32_t>& link_data{rocdata.data(half)};
    buffer_.insert(buffer_.end(), link_data.begin()+1, link_data.end());

    // ROC CRC Checksum
    buffer_.push_back(0xFFFF);
  }

  /** CRC Checksum computed by FPGA
   * We don't compute one right now, so just an extra word of all ones)
   */
  buffer_.push_back(0xFFFF);
}

void RawEventFile::endEvent() {
  /// Fill data tree
  data_tree_->Fill();

  /// clear raw data from memory
  buffer_.clear();
}

void RawEventFile::endRun() {
  runs_tree_->Fill();
}

}  // hexareformat
