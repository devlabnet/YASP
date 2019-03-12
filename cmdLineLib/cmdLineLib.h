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
//this following macro is good for debugging, e.g.  print2("myVar= ", myVar);
#define print2(x, y) (stream.print(x), stream.println(y))
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

//namespace CmdLineLibSpace {

const unsigned int CR = '\r';
const unsigned int LF = '\n';
const unsigned int BS = '\b';
const unsigned int NULLCHAR = '\0';
const unsigned int SPACE = ' ';
const unsigned int COMMAND_BUFFER_LENGTH = 32;

typedef void (*num_func)();
extern num_func functions[];

// Generic template
template<class T> 
inline Print &operator <<(Print &stream, T arg) 
{ stream.print(arg); return stream; }

class cmdLineLib : public Stream {
  public:
    cmdLineLib(Stream& dev, num_func* func, char** toks, size_t s);
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
   bool readOk() { return ok; }

  private:
    Stream &stream;
    int read();
    int available();
    int peek();
    size_t write(uint8_t b);
    num_func* userFunc;
    char** tokens;
    size_t userFuncSize;
  	char commandLine[COMMAND_BUFFER_LENGTH + 1]; //Read commands into this buffer from Serial.  +1 in length for a termination char
  	uint8_t charsRead = 0;						 //note: COMAND_BUFFER_LENGTH must be less than 255 chars long
  	bool ok;
    //const char *delimiters = ", \n"; //commands can be separated by return, space or comma
    const char *delimiters = " "; //commands must be separated by space
  	int isNumericString(char *s);
  	void nullCommand(char *ptrToCommandName);
  	void DoMyCommand();
};

//}

#endif /* CMDLINELIB_H_ */
