#include <yasplib.h>
yaspLib myPlot(Serial);

unsigned long loopMillis;
bool isLow = true;

void setup() {
  Serial.begin(115200);
  myPlot.Setup(0, "500mS", "#55ff7f");
  loopMillis = millis();
}

void loop() {
  if (millis() > loopMillis) {
      if (isLow) {
          myPlot.Data(0, 0);
          myPlot.Data(0, 1000);
      } else {
          myPlot.Data(0, 1000);
          myPlot.Data(0, 0);
      }
      isLow = ! isLow;
      loopMillis = millis() + 500;
  }
}
