#include "cmdLineLib.h"


CmdLineLibSpace::cmdLineLib cmdEngine(Serial);

void setup() {
  Serial.begin(115200);
  Serial.println("Start ...");

}

void loop() {
    cmdEngine.checkCommands();

}
