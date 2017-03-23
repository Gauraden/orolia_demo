#include "compressor.hpp"

class Collector {
  public:
    Collector();

    void UseCompressor(Compressor *ptr);
    void UseDataSource(VoidDataSource *ptr);
    Compressor::ShrPtr GetCompressor() const;
    bool GetDataHeader(VoidDataSource::Header *out) const;
  private:
    Compressor::ShrPtr     _comp;
    VoidDataSource::ShrPtr _source;
};