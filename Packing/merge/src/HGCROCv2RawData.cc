#include <HGCROCv2RawData.h>

#include <algorithm>
#include <iomanip>

HGCROCv2RawData::HGCROCv2RawData(int event, int chip, std::vector<uint32_t>::const_iterator data0_begin, std::vector<uint32_t>::const_iterator data0_end, std::vector<uint32_t>::const_iterator data1_begin, std::vector<uint32_t>::const_iterator data1_end) : m_data0(data0_begin, data0_end), m_data1(data1_begin, data1_end)
{
  m_chip=chip;
  
  m_event=event;
}

HGCROCv2RawData::HGCROCv2RawData(int event, int chip, const std::vector<uint32_t> &data0, const std::vector<uint32_t> &data1)
{
  m_chip=chip;
  
  m_event=event;

  std::copy( data0.begin(), data0.end(), std::back_inserter(m_data0) );

  std::copy( data1.begin(), data1.end(), std::back_inserter(m_data1) );
}

std::ostream& operator<<(std::ostream& out,const HGCROCv2RawData& rawdata)
{
  out << "event = " << std::dec << rawdata.m_event  << " " 
      << "chip = " << std::dec << rawdata.m_chip << std::endl;

  out << "first half : \n" ;;
  for( auto d : rawdata.m_data0 )
    out << "\t" << std::hex << std::setfill('0') << std::setw(8) << d ;
  out << std::endl;
  out << "second half : \n" ;;
  for( auto d : rawdata.m_data1 )
    out << "\t" << std::hex << std::setfill('0') << std::setw(8) << d ;
  out << std::endl;

  return out;
}

HGCROCEventContainer::HGCROCEventContainer()
{
  m_rawdata0=std::vector<uint32_t>(HGCROC_DATA_BUF_SIZE,0);
  m_rawdata1=std::vector<uint32_t>(HGCROC_DATA_BUF_SIZE,0);
  m_chip=0;
}


HGCROCEventContainer::HGCROCEventContainer(int chip) : HGCROCEventContainer()
{
  m_chip=chip;
}

void HGCROCEventContainer::fillContainer( int eventID, const std::vector<uint32_t>& half0, const std::vector<uint32_t>& half1 )
{
  m_mutex.lock();
  unsigned int len = std::min(half0.size(), half1.size())/HGCROC_DATA_BUF_SIZE;
  for( unsigned int iEvt = 0; iEvt < len; ++iEvt ){
    auto header0 = half0.begin() + iEvt * HGCROC_DATA_BUF_SIZE;
    auto header1 = half1.begin() + iEvt * HGCROC_DATA_BUF_SIZE;
    m_rocdata.emplace_back( new HGCROCv2RawData(eventID, m_chip, header0, header0+HGCROC_DATA_BUF_SIZE, header1, header1+HGCROC_DATA_BUF_SIZE) );
      eventID++;
  }
  m_mutex.unlock();
}
