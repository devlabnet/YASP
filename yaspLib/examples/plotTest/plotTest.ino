#include <yasplib.h>
yaspLib myPlot(Serial);

extern volatile unsigned long timer0_millis;
unsigned long new_value = 0;
void setMillis(unsigned long new_millis){
  uint8_t oldSREG = SREG;
  cli();
  timer0_millis = new_millis;
  SREG = oldSREG;
}
unsigned long loopMillis;
bool isW = true;
int sig = 0;
int cnt0 = 0;
double val;
double  pi = 3.14159 / 500.0;
double  pi2 = 3.14159 * 2.0 / 1000.0;

void setup() {
  Serial.begin(115200);
  Serial.println("Start ..");

//  myPlot.Setup(2, "Modulation", "#ffff00");
  myPlot.Setup(3, "ANALOG A5", "#66ffff");
  myPlot.Setup(0, "val", "#55ff7f");
  myPlot.Setup(1, "cos(val)", "#55ffff");
  myPlot.Setup(2, "sin(val)", "#ffff00");
  loopMillis = millis();
}

void loop() {
//  if (millis() > 30000) setMillis(0);
  if (cnt0 > 1000) cnt0 = 0;
//  ++cnt0;
  cnt0 += 1;
  val = pi2 * cnt0;
  myPlot.Data(0, val*100.0);
  myPlot.Data(1, cos(val)*1000.0);
  myPlot.Data(2, sin(val)*1000.0);
  myPlot.Data(3, analogRead(A5));
  
  if (millis() > loopMillis) {
    Serial.println("Hello");
      myPlot.Data(5, 500);
    if (isW) {
      myPlot.Color(1, "#ffffff");
    } else {
      myPlot.Color(1, "#ff5500");
    }
    isW = !isW;
    myPlot.Data(4, isW*1000);
    loopMillis = millis() + 500;
  }
}
