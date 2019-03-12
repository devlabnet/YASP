#include "cmdLineLib.h"


//cmdLineLib* cmdEngine;
USE_CMDLINELIB;

//******************************
// Add your commands function code here
//------------------------------
void setPfmVal() {
  Serial.print(F("-> PFM "));
  int v = CMD_GETINT;
    if (!CMD_OK) {
      v = -25;
    }
  Serial.println(v);
}
//------------------------------
void setInfoPeriod() {
  long p = CMD_GETLONG;
  if (!CMD_OK || (p < 100)) {
    p = 10000;
  }
  Serial.print(F("-> Info Period "));
  Serial.println(p);
}
//------------------------------
void test() {
  Serial.println("test ...");
}
//******************************
// Add your commands "token" and "function names" here
CMDS_LIST myCommands[] = {
                                {"pfm", setPfmVal},
                                {"per", setInfoPeriod},
                                {"t", test}
                              };
//******************************

void setup() {
  Serial.begin(115200);
  Serial.println("Start ...");
//  cmdEngine = new cmdLineLib(Serial, myCommands, NELEMS(myCommands));
  INIT_CMDS(Serial, myCommands);
//  functions[0]();
//  Serial.print(NELEMS(functions));
//  functions[1]();

}

void loop() {
  CHECK_CMDS;
//    cmdEngine->checkCommands();

}
