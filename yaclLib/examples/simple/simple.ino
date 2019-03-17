#include <yaclLib.h>
YACL_USE_YACLLIB;

//******************************
// Add your commands function code here
//------------------------------
void setPfmVal() {
    YACL_PRINT(F("-> PFM "));
    int v = YACL_GETINT;
    if (!YACL_OK) {
        v = -25;
    }
    YACL_PRINTLN(v);
}
//------------------------------
void setInfoPeriod() {
    long p = YACL_GETLONG;
    if (!YACL_OK || (p < 100)) {
        p = 10000;
    }
    YACL_PRINT(F("-> Info Period "));
    YACL_PRINTLN(p);
}
//------------------------------
void test() {
    YACL_PRINT("test --> ");
    char* c = YACL_GETSTR;
    int v = YACL_GETINT;
    YACL_PRINT(c);
    YACL_PRINT("=");
    YACL_PRINTLN(v);
    YACL_WRITE(v);
    YACL_PRINTLN();
}
//******************************
// Add your commands "token" and "function names" here
YACL_CMDS_LIST myCommands[] = {
    {"pfm", setPfmVal},
    {"per", setInfoPeriod},
    {"t", test}
};
//******************************

void setup() {
    Serial.begin(115200);
    YACL_INIT_CMDS(Serial, myCommands);
    YACL_PRINTLN("Start ...");
}

void loop() {
    YACL_CHECK_CMDS;
}
