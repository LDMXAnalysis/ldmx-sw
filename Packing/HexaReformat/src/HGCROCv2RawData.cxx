#include <Packing/HexaReformat/HGCROCv2RawData.h>

#include <algorithm>
#include <iomanip>

namespace packing {
namespace hexareformat {

std::ostream& operator<<(std::ostream& out, const HGCROCv2RawData& rawdata) {
  out << "event = " << std::dec << rawdata.m_event << " "
      << "chip = " << std::dec << rawdata.m_chip << std::endl;

  out << "first half : \n";
  ;
  for (auto d : rawdata.m_data0)
    out << "\t" << std::hex << std::setfill('0') << std::setw(8) << d;
  out << std::endl;
  out << "second half : \n";
  ;
  for (auto d : rawdata.m_data1)
    out << "\t" << std::hex << std::setfill('0') << std::setw(8) << d;
  out << std::endl;

  return out;
}

}  // hexareformat
}  // packing
