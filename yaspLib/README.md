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
<h3>Predefined MACROS</h3>
To ease coding, some <strong>Predefined Macros</strong> are available:
<ul>
    <li>Get Numeric Value (if no numeric is available or if not a well formatted, YACL_OK will return false):</li>
    <ul>
    <li><strong>YACL_GETINT</strong>: Return the next INT in the command Line</li>
    <li><strong>YACL_GETLONG</strong>: Return the next LONG in the command Line</li>
    <li><strong>YACL_GETFLOAT</strong>: Return the next FLOAT in the command Line</li>
    <li><strong>YACL_OK</strong>: Return false if expected number is not found or is not well formatted.</li>
    </ul>
    <li>Get String Value:</li>
    <ul>
    <li><strong>YACL_GETSTR</strong>: Return the next STRING in the command Line</li>
    </ul>
    <li>Print/Write:</li>
    <ul>
    <li><strong>YACL_PRINT(x)</strong>: Print x value, this is the same as Serial.print(x)</li>
    <li><strong>YACL_PRINTLN(x)</strong>: same as Serial.println(x)</li>
    <li><strong>YACL_PRINT2(x, y)</strong>: Print x value in specific format, this is the same as Serial.print(x, FORMAT)</li>
    <li><strong>YACL_PRINTLN2(x, y)</strong>: same as Serial.println(x, FORMAT)</li>
    <li><strong>YACL_WRITE(x)</strong>: Writes binary data to the serial port</li>
    </ul>
    <li>Last Token:</li>
    <ul>
    <li><strong>YACL_TOKEN</strong>: Return the last command line token found</li>
    </ul>
</ul>


