#ifndef HGCROCV2RAWDATA
#define HGCROCV2RAWDATA 1

#include <iostream>
#include <deque>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/thread/thread.hpp>

#define HGCROC_DATA_BUF_SIZE 43  //36 channels + 2 common mode gathered together + 1 calib + 1 header + 4 iddles in trailer per half

class HGCROCv2RawData
{
 public:
  HGCROCv2RawData(){;}

  HGCROCv2RawData(int event, int chip, const std::vector<uint32_t> &data0, const std::vector<uint32_t> &data1);
  HGCROCv2RawData(int event, int chip, 
		  std::vector<uint32_t>::const_iterator data0_begin, std::vector<uint32_t>::const_iterator data0_end, 
		  std::vector<uint32_t>::const_iterator data1_begin, std::vector<uint32_t>::const_iterator data1_end);
  
  int event() const { return m_event; }
  
  int chip() const { return m_chip; }
  
  const std::vector<uint32_t> &data(int chiphalf) const { return chiphalf==0 ? m_data0 : m_data1; }
  
  friend std::ostream& operator<<(std::ostream& out,const HGCROCv2RawData& h);

 private:
  friend class boost::serialization::access;
  template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & m_event;
      ar & m_chip;
      ar & m_data0;
      ar & m_data1;
    }

 private:
  int m_event;
  int m_chip;
  std::vector<uint32_t> m_data0;
  std::vector<uint32_t> m_data1;
  
};

class HGCROCEventContainer
{
 public:
  HGCROCEventContainer();
  HGCROCEventContainer(int chip);
  
  void fillContainer( int eventID, const std::vector<uint32_t>& half0, const std::vector<uint32_t>& half1 );

  inline std::deque< std::unique_ptr<HGCROCv2RawData> >& getDequeEvents() {return m_rocdata;} 
  inline void deque_lock(){ m_mutex.lock(); }
  inline void deque_unlock(){ m_mutex.unlock(); }
   
 private:
  std::deque< std::unique_ptr<HGCROCv2RawData> > m_rocdata;
  boost::mutex m_mutex;
  std::vector<uint32_t> m_rawdata0;
  std::vector<uint32_t> m_rawdata1;
  int m_chip;
};

#endif
