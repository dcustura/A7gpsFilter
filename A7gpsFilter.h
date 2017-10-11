#ifndef A7gpsFilter_h
#define A7gpsFilter_h

#include "Arduino.h"

class A7GPSFilter : public Stream {
  private:
    Stream &serial;
    typedef void (*Callback)(uint8_t);
    Callback callback;
    enum {
      BOL, // beginning of line
      LINE_CONTENT,
      LINE_END_CR,
      BOL_CR, // CR at the beginning of line
      CR_LINE_CONTENT,
      GPS_HEAD,
      GPS_HEAD_PUSHBACK_CRLF,
      GPS_HEAD_PUSHBACK_LF,
      GPS_HEAD_PUSHBACK,
      GPS_BOL,
      GPS_LINE_CONTENT,
      GPS_LINE_END_CR,
    } recvState = BOL;
    uint8_t headIdx = 0;
    uint8_t tailIdx = 0;
    uint8_t lastCharRead;
    void pullGPSData();
  public:
    A7GPSFilter(Stream &a7Serial, Callback callback) : serial(a7Serial), callback(callback) {}
    void gpsOn();
    void gpsOff();
    // methods inherited from Stream class:
    virtual size_t write(uint8_t character);
    virtual int available();
    virtual int read();
    virtual int peek();
};

#endif
