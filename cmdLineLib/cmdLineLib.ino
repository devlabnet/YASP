#include "cmdLineLib.h"


cmdLineLib* cmdEngine;
//******************************
// Add your commands here
//******************************
void setPfmVal() {
  Serial.print(F("-> PFM "));
  Serial.println(cmdEngine->readNumber());
}
//------------------------------
void setInfoPeriod() {
  long p = cmdEngine->readNumber();
  if ((cmdEngine->readOk() == false) || (p < 100)) {
    p = 10000;
  }
  Serial.print(F("-> Info Period "));
  Serial.println(p);
}
//------------------------------
void test() {
  Serial.println("test ...");
}

num_func functions[] = {setPfmVal, setInfoPeriod, test};
char* tokens[] = {"pfm", "per", "t"};
//******************************

void setup() {
  Serial.begin(115200);
  Serial.println("Start ...");
  cmdEngine = new cmdLineLib(Serial, functions, tokens, NELEMS(functions));
//  functions[0]();
//  Serial.print(NELEMS(functions));
//  functions[1]();

}

void loop() {
    cmdEngine->checkCommands();

}
