
#include "Packing/Unpacker.h"

#include "TFile.h"
#include "TTree.h"

namespace packing {

void Unpacker::configure(framework::config::Parameters& ps) {
  // create the configured translators
  Processor::configure(ps);

  raw_file_ = ps.getParameter<std::string>("raw_file");
  raw_tree_ = ps.getParameter<std::string>("raw_tree");
  skip_unavailable_ = ps.getParameter<bool>("skip_unavailable");
}

void Unpacker::onProcessStart() {
  // open file and get tree of raw data
  file_ = TFile::Open(raw_file_.c_str());
  tree_ = (TTree*)file_->Get(raw_tree_.c_str());
  i_entry_ = -1;

  // go through list of branches
  auto branch_listing{tree_->GetListOfBranches()};
  for (int i_br{0}; i_br < branch_listing->GetEntriesFast(); i_br++) {
    auto br{(TBranch*)branch_listing->UncheckedAt(i_br)};
    std::string br_name{br->GetName()};
    auto optional_translator{getTranslator(br_name)};
    if (optional_translator) {
      unpackers_.emplace_back(br, *optional_translator);
    } else if (skip_unavailable_) {
      ldmx_log(info) << "Unable to find a translator for '" 
        << br_name << "', skipping...";
    } else {
      EXCEPTION_RAISE("DeduceTranslator",
          "Unable to find a translator that can translate '"+br_name+"'.");
    }
  }
}

void Unpacker::produce(framework::Event& event) {
  if (i_entry_+1 > tree_->GetEntriesFast()) {
    /// can we turn this into end run?
    abortEvent();
  }

  i_entry_++;
  tree_->GetEntry(i_entry_);

  for (auto& unpacker : unpackers_)
    unpacker.decode(event);
}

void Unpacker::onProcessEnd() {
  file_->Close();
  tree_ = nullptr;
  i_entry_ = -2;
}

Unpacker::SingleUnpacker::SingleUnpacker(TBranch* br, TranslatorPtr t) : translator_{t} {
  buffer_ = new BufferType;
  br->SetAddress(&buffer_);
}

void Unpacker::SingleUnpacker::decode(framework::Event& e) {
  translator_->decode(e, *buffer_);
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Unpacker)
