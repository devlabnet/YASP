#include <yaclLib.h>
USE_YACLLIB;

//******************************
// Add your commands function code here
//------------------------------
void setPfmVal() {
    YACL_PRINT(F("-> PFM "));
    int v = CMD_GETINT;
    if (!CMD_OK) {
        v = -25;
    }
    YACL_PRINTLN(v);
}
//------------------------------
void setInfoPeriod() {
    long p = CMD_GETLONG;
    if (!CMD_OK || (p < 100)) {
        p = 10000;
    }
    YACL_PRINT(F("-> Info Period "));
    YACL_PRINTLN(p);
}
//------------------------------
void test() {
    YACL_PRINT("test --> ");
    char* c = CMD_GETSTR;
    int v = CMD_GETINT;
    YACL_PRINT(c);
    YACL_PRINT("=");
    YACL_PRINTLN(v);
    YACL_WRITE(v);
    YACL_PRINTLN();
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
    INIT_CMDS(Serial, myCommands);
    YACL_PRINTLN("Start ...");
}

void loop() {
    CHECK_CMDS;
}
