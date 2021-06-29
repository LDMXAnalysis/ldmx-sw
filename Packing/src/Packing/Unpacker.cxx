
#include "Packing/Unpacker.h"

namespace packing {

void Unpacker::configure(framework::config::Parameters& ps) {
  // create the configured translators
  Processor::configure(ps);

  raw_file_ = ps.getParameter<std::string>("raw_file");
}

void Unpacker::onProcessStart() {
  file_ = TFile::Open(raw_file_);
  tree_ = (TTree*)file_->Get(raw_tree_);
  tree_->SetBranchAddress(raw_name_, &raw_data_);
  i_entry_ = -1;
}

void Unpacker::produce(framework::Event& event) {
  if (i_entry_+1 > tree_->GetEntriesFast()) {
    /// can we turn this into end run?
    abortEvent();
  }

  i_entry_++;
  tree_->GetEntry(i_entry_);

  for (const auto&[name, buffer] : raw_data_) {
    getTranslator(name)->decode(event, buffer);
  }  // loop over raw data packages
}

void Unpacker::onProcessEnd() {
  file_->Close();
  tree_ = nullptr;
  i_entry_ = -2;
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Unpacker)
