#include <yasplib.h>
#include <yaclLib.h>

yaspLib myPlot(Serial);

unsigned long loopMillisAmp;
unsigned long loopMillisPeriod;
unsigned long milli;
bool isLowF = true;
bool isLowA = true;
int freq = 1;
int period = 500;
int modPeriod = 500;
int amp = 1000;
int modAmp = 1000;
int mod = 100;
int modDelta = 0;
float modFactor = 1.0;
int cntPer = 0;
int cntAmp = 0;
double val;
double  pi = 3.14159 / 50.0;

YACL_USE_YACLLIB;
//******************************
// Add your commands function code here
//------------------------------
void setFrequence() {
  YACL_PRINT(F("-> F "));
  int v = YACL_GETINT;
  if (!YACL_OK || v < 0) {
    v = 1;
  }
  if (v > 100) v = 100;
  freq = constrain(v, 1, 100);
  YACL_PRINT(v);
  freq = v;
  period = 500 / v;
  YACL_PRINT(" / period: ");
  YACL_PRINT(1.0 / v);
  YACL_PRINTLN(" Sec");
}

//------------------------------
void setAmplitude() {
  YACL_PRINT(F("-> A "));
  int v = YACL_GETINT;
  if (!YACL_OK || v < 0 || v > 1000) {
    v = 1000;
  }
  YACL_PRINTLN(v);
  amp = v;
}

//------------------------------
void modulation() {
  YACL_PRINT(F("-> M "));
  int m = YACL_GETINT;
  if (!YACL_OK || m < 0 || m > 100) {
    m = 100;
  }
  modFactor = m / 100.0;
  YACL_PRINTLN(m);
  mod = m;
}

//------------------------------
void halt() {
}

//******************************
// Add your commands "token" and "function names" here
YACL_CMDS_LIST myCommands[] = {
  {"F", setFrequence},
  {"A", setAmplitude},
  {"S", halt},
  {"M", modulation}
};
//******************************

void setup() {
  Serial.begin(115200);
  YACL_INIT_CMDS(Serial, myCommands);
  myPlot.Setup(0, "Frequency", "#55ff7f");
  myPlot.Setup(1, "Amplitude", "#ff55d5");
  loopMillisAmp = millis();
  loopMillisPeriod = millis();
}

void loop() {
  YACL_CHECK_CMDS;
  //    Serial.println(millis() - micro);
  milli = millis();
  if (YACL_TOKEN == "S") {
  } else {
    if (milli >= loopMillisPeriod) {
      loopMillisPeriod = milli + modPeriod;
      if (isLowF) {
        myPlot.DataMillis(0, milli, -1000);
        if (YACL_TOKEN == "M") {
          if (cntPer > 100) cntPer = 0;
          val = pi * cntPer;
          ++cntPer;
          int f = 50 * modFactor * abs(cos(val));
          //          int f = freq * modFactor * abs(cos(val));
          f = constrain(f, 1, 100);
          modPeriod = 500 / f;
          //          Serial.print(freq);
          //          Serial.print(" ");
          //          Serial.print(f);
          //          Serial.print(" ");
          //          Serial.println(modPeriod);
        } else {
          modPeriod = 500 / freq;
        }
        myPlot.DataMillis(0, milli, 1000);
      } else {
        myPlot.DataMillis(0, milli, 1000);
        myPlot.DataMillis(0, milli, -1000);
      }
      isLowF = ! isLowF;
    }
    if (millis() > loopMillisAmp) {
      if (isLowA) {
        myPlot.Data(1, -modAmp);
        if (YACL_TOKEN == "M") {
          if (cntAmp > 100) cntAmp = 0;
          val = pi * cntAmp;
          ++cntAmp;
          modAmp = amp * modFactor * cos(val);
        } else {
          modAmp = amp;
        }
        myPlot.Data(1, modAmp);
      } else {
        myPlot.Data(1, modAmp);
        myPlot.Data(1, -modAmp);
      }
      isLowA = ! isLowA;
      loopMillisAmp = millis() + 50;
    }
  }
}
