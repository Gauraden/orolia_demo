#include "compressor.hpp"

class Collector {
  public:
    Collector();

    void UseCompressor(Compressor *ptr);
    void UseDataSource(VoidDataSource *ptr);
    Compressor::ShrPtr GetCompressor() const;
    bool GetDataHeader(VoidDataSource::Header *out) const;
    bool Begin();
    bool FetchAllRecords();
    void End();
    std::string GetErrorMessage() const;
  private:
    Compressor::ShrPtr     _comp;
    VoidDataSource::ShrPtr _source;
};