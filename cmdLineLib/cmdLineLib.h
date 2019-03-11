/*
 * CmdLineLib.h
 *
 *  Created on: 11 03 2019
 *      Author: Cricri042
 */
#ifndef CMDLINELIB_H_
#define CMDLINELIB_H_
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
#define print2(x, y) (stream.print(x), stream.println(y))

namespace CmdLineLibSpace {
//this following macro is good for debugging, e.g.  print2("myVar= ", myVar);

const unsigned int CR = '\r';
const unsigned int LF = '\n';
const unsigned int BS = '\b';
const unsigned int NULLCHAR = '\0';
const unsigned int SPACE = ' ';
const unsigned int COMMAND_BUFFER_LENGTH = 32;

// Generic template
template<class T> 
inline Print &operator <<(Print &stream, T arg) 
{ stream.print(arg); return stream; }

class cmdLineLib : public Stream {
  public:
    cmdLineLib(Stream& dev);
    ~cmdLineLib();
  	/***************************************************************************
  	    getCommandLineFromSerialPort()
  	      Return the string of the next command. Commands are delimited by return"
  	      Handle BackSpace character
  	      Make all chars lowercase
  	****************************************************************************/
  	bool getCommandLineFromSerialPort();
  	int readNumber();
  	long readLong();
  	char *readWord();
  	void checkCommands();
  	/****************************************************
  	   Add your commands here
  	*/
  	void setPfmVal();
  	void setInfoPeriod();
  	void setMinPulseWidth();
  	void doPwm();

  private:
    Stream &stream;
    int read();
    int available();
    int peek();
    size_t write(uint8_t b);
  	char commandLine[COMMAND_BUFFER_LENGTH + 1]; //Read commands into this buffer from Serial.  +1 in length for a termination char
  	uint8_t charsRead = 0;						 //note: COMAND_BUFFER_LENGTH must be less than 255 chars long
  	bool ok;
    //const char *delimiters = ", \n"; //commands can be separated by return, space or comma
    const char *delimiters = " "; //commands must be separated by space
  	int isNumericString(char *s);
  	void nullCommand(char *ptrToCommandName);
  	void DoMyCommand();
 
  	/*************************************************************************************************************
  	     your Command Names Here
  	*************************************************************************************************************/
  	const char *robotWaitCmdToken = "$";
  	const char *lnfoPeriodCmdToken = "i";
  	const char *minPulseWidthCmdToken = "p";
  	const char *pwmCmdToken = "w";
};

}

#endif /* CMDLINELIB_H_ */
