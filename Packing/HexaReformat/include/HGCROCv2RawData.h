#ifndef PACKING_HEXAREFORMAT_HGCROCV2RAWDATA
#define PACKING_HEXAREFORMAT_HGCROCV2RAWDATA

#include <iostream>
#include <deque>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

/**
 * The HGCROC Data buffer size is fixed by the construction of the hardware.
 * 36 channels + 2 common mode gathered together + 1 calib + 1 header 
 *  + 4 iddles in trailer per half
 */
#define HGCROC_DATA_BUF_SIZE 43 

/**
 * @namespace hexareformat
 *
 * Helper classes for interfacing with raw data.
 * The code in this namespace is not expected to be around long.
 */
namespace hexareformat {

/**
 * This HGCROCv2RawData class is copied almost exactly from 
 * the hexactrl-sw borrowed from CMS. I have deleted the parts
 * of it that are used to write the raw data coming from the HGC ROC
 * in-order-to cleanup the code.
 *
 * Each instance of this class represents an individual sample 
 * from each channel on both halves of the ROC.
 */
class HGCROCv2RawData {
 public:
  HGCROCv2RawData() {}

  int event() const { return m_event; }

  int chip() const { return m_chip; }

  const std::vector<uint32_t>& data(int chiphalf) const {
    return chiphalf == 0 ? m_data0 : m_data1;
  }

  friend std::ostream& operator<<(std::ostream& out, const HGCROCv2RawData& h);

 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& m_event;
    ar& m_chip;
    ar& m_data0;
    ar& m_data1;
  }

 private:
  int m_event;
  int m_chip;
  std::vector<uint32_t> m_data0;
  std::vector<uint32_t> m_data1;
};  // HGCROCv2RawData

}  // hexareformat

#endif
