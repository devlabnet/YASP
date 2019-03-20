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

```c++
#include <yaclLib.h>
YACL_USE_YACLLIB;

bool getOperands(float& op1, float& op2) {
  op1 = YACL_GETFLOAT;
  if (!YACL_OK) {
    YACL_PRINTLN(F("Mising first operand ! "));
    return false;
  }
  op2 = YACL_GETFLOAT;
  if (!YACL_OK) {
    YACL_PRINTLN(F("Mising second operand ! "));
    return false;
  }
  YACL_PRINT2(op1, 5);
  YACL_PRINT(F(" and "));
  YACL_PRINT2(op2, 5);
  return true;
}

//******************************
// Add your commands function code here
//------------------------------
void addition() {
  float op1, op2;
  YACL_PRINT(F("-> ADD "));
  if (getOperands(op1, op2)) {
    YACL_PRINT(F(" = "));
    YACL_PRINTLN2(op1 + op2, 5);
  }
}
//------------------------------
void substraction() {
  float op1, op2;
  YACL_PRINT(F("-> SUB "));
  if (getOperands(op1, op2)) {
    YACL_PRINT(F(" = "));
    YACL_PRINTLN2(op1 - op2, 5);
  }
}
//------------------------------
void division() {
  float op1, op2;
  YACL_PRINT(F("-> DIV "));
  if (getOperands(op1, op2)) {
    YACL_PRINT(F(" = "));
    YACL_PRINTLN2(op1 / op2, 5);
  }
}
//------------------------------
void multiplication() {
  float op1, op2;
  YACL_PRINT(F("-> MULT "));
  if (getOperands(op1, op2)) {
    YACL_PRINT(F(" = "));
    YACL_PRINTLN2(op1 * op2, 5);
  }
}
//------------------------------
void help() {
  YACL_PRINTLN(F("-----------------------------------"));
  YACL_PRINTLN(F("Enter op OP1 OP2"));
  YACL_PRINTLN(F("  -> where OP1 and OP2 are float"));
  YACL_PRINTLN(F("  -> and op is one of the following operation:"));
  YACL_PRINTLN(F("      + : Addition"));
  YACL_PRINTLN(F("      - : Substraction"));
  YACL_PRINTLN(F("      * : Multiplication"));
  YACL_PRINTLN(F("      / : Division"));
  YACL_PRINTLN(F("-----------------------------------"));
}
//******************************
// Add your commands "token" and "function names" here
YACL_CMDS_LIST myCommands[] = {
  {"+", addition},
  {"-", substraction},
  {"*", multiplication},
  {"/", division},
  {"?", help}
};
//******************************

void setup() {
  Serial.begin(115200);
  YACL_INIT_CMDS(Serial, myCommands);
  // Show HELP
  help();
}

void loop() {
  YACL_CHECK_CMDS;
}
```
