#include <yasplib.h>
yaspLib myPlot(Serial);
unsigned long loopMillis;
int val = 500;

void setup() {
    // Start Serial
    Serial.begin(115200);
    myPlot.Setup(0, "Triangle wave 0", "#ffff00");
    myPlot.Setup(1, "Triangle wave Millis", "#ff5555");
    Serial.println("Hello");
    loopMillis = millis();
}

void loop() {
    if (millis() >= loopMillis) {
        myPlot.DataMillis(1, loopMillis, val);
        loopMillis = millis() + 50;
        myPlot.Data(0, val);
        val = -val;
    }
}
