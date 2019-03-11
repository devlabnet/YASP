#include "yasplib.h"

/************************************************/
yaspLib::yaspLib(Stream& dev) : stream(dev) {
}
/************************************************/
int yaspLib::read() {
  return stream.read();
}
/************************************************/
int yaspLib::available() {
  return stream.available();
}
/************************************************/
int yaspLib::peek() {
  return stream.peek();
}
/************************************************/
size_t yaspLib::write(uint8_t b) {
  return stream.write(b);
}
/************************************************/
void yaspLib::Setup(int ind, char* name, char* color) {
  stream.write(PLOT_MSG);
  stream.print(ind);
  stream.write(SPACE_MSG);
  stream.print(name);
  stream.write(SPACE_MSG);
  stream.print(color);
  stream.write(END_MSG);
}
/************************************************/
void yaspLib::Color(int ind, char* color) {
  stream.write(PLOT_MSG);
  stream.print(ind);
  stream.write(SPACE_MSG);
  stream.print(color);
  stream.write(END_MSG);
}
/************************************************/
void yaspLib::Data(int ind, double val) {
  stream.write(START_MSG);
  stream.print(ind);
  stream.write(SPACE_MSG);
  stream.print(millis());
  stream.write(SPACE_MSG);
  stream.print(val);
  stream.write(END_MSG);
}
