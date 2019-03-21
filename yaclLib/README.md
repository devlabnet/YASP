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
Then Define the functions to call for each command TOKEN"
```c++
//******************************
// Add your commands function code here
//------------------------------
void myCommandTest() {
    // Here the command code to run
    // .....
}
```
Then Add your commands "token" and corresponding  "function names"
```c++
//******************************
// Add your commands "token" and "function names" here
//------------------------------
YACL_CMDS_LIST myCommands[] = {
    {"Test", myCommandTest}
};
//******************************
```
Then init the commanf line in your setup() function
and Check for any commands from the serial in the loop function
```c++
void setup() {
    Serial.begin(115200);
    // INIT THE COMMANDS
    YACL_INIT_CMDS(Serial, myCommands);
}

void loop() {
    // Check for commands as often as possible
    YACL_CHECK_CMDS;
}
```

