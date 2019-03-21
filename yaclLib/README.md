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
    <li><strong>Get Number</strong> (if no number is available or if not a good number format, YACL_OK will return false) :</li>
    <ul>
    <li><strong>YACL_GETINT</strong>: Return the next INT in the command Line</li>
    <li><strong>YACL_GETLONG</strong>: Return the next LONG in the command Line</li>
    <li><strong>YACL_GETFLOAT</strong>: Return the next FLOAT in the command Line</li>
    </ul>
    <li><strong>YACL_GETSTR</strong>: Return the next STRING in the command Line</li>
    </ul>
</ul>

#define YACL_GETINT ENGINE->readNumber()
#define YACL_GETLONG ENGINE->readLong()
#define YACL_GETFLOAT ENGINE->readFloat()
#define YACL_GETSTR ENGINE->readWord()
#define YACL_OK ENGINE->readOk()
#define YACL_PRINT(x) ENGINE->stream.print(x)
#define YACL_PRINTLN(x) ENGINE->stream.println(x)
#define YACL_PRINT2(x, y) ENGINE->stream.print(x, y)
#define YACL_PRINTLN2(x, y) ENGINE->stream.println(x, y)
#define YACL_WRITE(x) ENGINE->stream.write(x)
#define YACL_TOKEN ENGINE->token()

