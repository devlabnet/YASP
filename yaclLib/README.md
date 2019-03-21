[![Build Status](https://travis-ci.org/pvizeli/CmdParser.svg?branch=master)](https://travis-ci.org/pvizeli/CmdParser)

# YACL Yet Another Command Line
A simple and most powerfull cmd parser.

This library is for handling commands over i.e. serial and short data transfers.

For handling show all examples.

## Getting Started

To start using YACL, just include the library header and initialize the Command Line Engine "YACL_USE_YACLLIB" in your sketch file :
```c++
#include <yaclLib.h>
YACL_USE_YACLLIB;
```
Then Define the functions to call for each <strong>"command TOKEN"</strong>
```c++
//******************************
// Add your commands function code here
//------------------------------
void myCommandTest() {
    // Here the command code to run when receiving the "Test" token
    // .....
}
```
Then Add your commands <strong>"token"</strong> and related <strong>"function names"</strong>
```c++
//******************************
// Add your commands "token" and "function names" here
//------------------------------
YACL_CMDS_LIST myCommands[] = {
    {"Test", myCommandTest}
};
//******************************
```
Then init the command line in your <strong>setup()</strong> function
and Check (as often as possible) for any commands from the serial in the <strong>loop()</strong> function
```c++
void setup() {
    Serial.begin(115200);
    // INIT THE COMMANDS
    YACL_INIT_CMDS(Serial, myCommands);
}

void loop() {
    // CHECK FOR COMMANDS AS OFTEN AS POSSIBLE
    YACL_CHECK_CMDS;
}
```
For a more complex exemple, look at the <strong>"Calculator"</strong> in the examples. All samples are heavily documented and should be self explanatory.
<h3>Predifined MACROS</h3>
To ease coding, some <strong>Predifined Macros</strong> are available:
<ul>
    <li>Get Numeric Value (if no numeric is available or if not a well formated, YACL_OK will return false):</li>
    <ul>
    <li><strong>YACL_GETINT</strong>: Return the next INT in the command Line</li>
    <li><strong>YACL_GETLONG</strong>: Return the next LONG in the command Line</li>
    <li><strong>YACL_GETFLOAT</strong>: Return the next FLOAT in the command Line</li>
    <li><strong>YACL_OK</strong>: Return false if expected number is not found or is not well formated</li>
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
    <li><strong>YACL_TOKEN</strong>: Retuen the last command line token found</li>
    </ul>
</ul>


