#ifndef YASPLIB_H
#define YASPLIB_H
#include <Arduino.h>

class yaspLib : public Stream {
  private:
   Stream &stream;
   enum MsgType {
        START_MSG = 0x10,
        PLOT_MSG = 0x11,
        END_MSG = 0x12,
        SPACE_MSG = 0x13
    };
   int read();
   int available();
   int peek();
   size_t write(uint8_t b);
  public:
    yaspLib(Stream& dev);
    void Setup(int ind, char* name, char* color);
    void Color(int ind, char* color);
    void Data(int ind, double val);
};

#endif // YASPLIB_H