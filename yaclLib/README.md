[![Build Status](https://travis-ci.org/pvizeli/CmdParser.svg?branch=master)](https://travis-ci.org/pvizeli/CmdParser)

# YACL Yet Another Command Line
A simple and most powerfull cmd parser.

This library is for handling commands over i.e. serial and short data transfers.

For handling show all examples.

## Parser Object

The function ```parseCmd``` will change the buffer! Only the parser object can
handle the new buffer...

Default it use a static parameter list. With KeyValue option you can change
to dynamic key=value parser handling.

```c++
#include <CmdParser.hpp>

CmdParser myParser;

myParser.parseCmd(myBuffer); // CmdBuffer Object
myParser.parseCmd(byteBuffer, size); // Binary buffer
myParser.parseCmd(cString); // C string buffer

```

### Options
- ```setOptIgnoreQuote``` (default off) support string with "my value"
- ```setOptSeperator``` (default ' ') use this character for seperate cmd
- ```setOptKeyValue``` (default off) Support dynamic key=value feature


## Buffer Object

```c++
#include <CmdBuffer.hpp>

CmdBuffer<32> myBuffer;

// Reading Data
myBuffer.readFromSerial(&Serial, numTimeout);
```

### Options
- ```setEndChar``` (default '\n') set character for stop reading. Normal is a line end.

## Callback Object

```c++
#include <CmdCallback.hpp>

CmdCallback<5> myCallback; // Object for handling 5 function in SRAM
CmdCallback_P<5> myCallbackP; // Object for handling 5 function in PROGMEM

// add function
myCallbackP.addCmd(PSTR("SET"), &myFunctForSet);

// Automatic proccessing
myCallbackP.loopCmdProcessing(&myParser, &myBuffer, &Serial);

// Manual
myCallbackP.processCmd(cstrCmd);
```
