#include <yasplib.h>
#include <yaclLib.h>

// Create a Plotter object
yaspLib myPlot(Serial);
int mult = 1;

// Init Command Line engine
YACL_USE_YACLLIB;

//******************************
// Add your commands function code here
//******************************
void sendValue() {
    int v = YACL_GETINT;
    if (!YACL_OK) {
      return;
    }
    YACL_PRINT(F("-> Send Value: "));
    YACL_PRINT(v);
    YACL_PRINT(F(" / Factor: "));
    YACL_PRINTLN(mult);
    myPlot.Data(0, v * mult);
}

void setFactor() {
    int m = YACL_GETINT;
    if (!YACL_OK) {
      m = 1;
    }
    YACL_PRINT(F("-> Factor Set to: "));
    YACL_PRINTLN(m);
    mult = m;
}

//******************************
// Add your commands "token" and "function names" here
//******************************
YACL_CMDS_LIST myCommands[] = {
    {"Val", sendValue},
    {"*", setFactor}
};


void setup() {
    // Start Serial
    Serial.begin(115200);
    // Init Command Line tokens / functions
    YACL_INIT_CMDS(Serial, myCommands);

    // Set name and color for the plot 0
    myPlot.Setup(0, "Value", "#55ff7f");
}

void loop() {
    // Checks Commands (Serial Read())
    YACL_CHECK_CMDS;
    // Your prefered code Here
    // ....
}
