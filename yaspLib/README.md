[![Build Status](https://travis-ci.org/pvizeli/CmdParser.svg?branch=master)](https://travis-ci.org/pvizeli/CmdParser)

# YASP Yet Another Serial Plot
The easiest way to send data to the YASP Application.

## Getting Started

To start using YASP, just include the library header "<yaspLib.h>" and create a "Plotter" Object in your sketch file :
```c++
#include <yasplib.h>
yaspLib myPlot(Serial);
```
unsigned long plotMillis;
int val = 500;

Then optionally, add any (up to 10) "plots" description in your <strong>setup()</strong> function
and send data for this plot in the <strong>loop()</strong> function
```c++
void setup() {
    Serial.begin(115200);
    // ADD a Plot description: Indice = 0, Name = "Triangle wave 0" and Color = yellow.
    myPlot.Setup(0, "Triangle wave 0", "#ffff00");
    plotMillis = millis();
}

void loop() {
    if (millis() >= plotMillis) {
        // Send Data to plot 0
        myPlot.Data(0, val);
        val = -val;
        plotMillis = millis() + 50;
    }
}
```
For a more complex exemple, look at samples in the examples directory. All samples are heavily documented and should be self explanatory.
<h3>yaspLib Class Reference</h3>
<h4>Public Member Functions</h4>
<ul>
<li>
yaspLib (Stream &dev)
</li>
<li>
void Setup (int ind, char *name, char *color)
</li>
<li>
void Color (int ind, char *color)
</li>
<li>
void Data (int ind, double val)
</li>
<li>
void DataMicros (int ind, unsigned long m, double val)
</li>
<li>
void DataMillis (int ind, unsigned long m, double val)
</li>
</ul>
