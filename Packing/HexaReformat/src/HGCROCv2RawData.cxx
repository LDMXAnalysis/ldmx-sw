#include "HGCROCv2RawData.h"

#include <algorithm>
#include <iomanip>

namespace hexareformat {

std::ostream& operator<<(std::ostream& out, const HGCROCv2RawData& rawdata) {
  out << "event = " << std::dec << rawdata.m_event << " "
      << "chip = " << std::dec << rawdata.m_chip << std::endl;

  out << "first half :";
  for (auto d : rawdata.m_data0)
    out << "  " << std::hex << std::setfill('0') << std::setw(8) << d;
  out << std::endl;
  out << "second half :";
  for (auto d : rawdata.m_data1)
    out << "  " << std::hex << std::setfill('0') << std::setw(8) << d;
  out << std::dec << std::endl;

  return out;
}

}  // hexareformat
