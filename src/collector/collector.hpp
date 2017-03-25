#ifndef COLLECTOR_HPP
#define COLLECTOR_HPP

#include "compressor.hpp"

class Collector {
  public:
    typedef std::list<std::string> Messages;
    Collector();

    void UseCompressor(Compressor *ptr);
    void UseDataSource(VoidDataSource *ptr);
    Compressor::ShrPtr GetCompressor() const;
    bool GetDataHeader(VoidDataSource::Header *out) const;
    bool Begin();
    bool FetchAllRecords();
    void End();
    const Messages& GetMessages() const;
  private:
    void RegisterMessage(const std::string &msg);
    Compressor::ShrPtr     _comp;
    VoidDataSource::ShrPtr _source;
    Messages               _messages;
};
#endif