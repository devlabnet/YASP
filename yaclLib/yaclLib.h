/*
 * yaclLib.h
 *
 *  Created on: 11 03 2019
 *      Author: Cricri042
 */
#ifndef YACLLIB_H_
#define YACLLIB_H_
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <Arduino.h>
//this following macro is good for debugging, e.g.  print2("myVar= ", myVar);
#define print2(x, y) (stream.print(x), stream.println(y))
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define ENGINE cmdEngine
#define USE_YACLLIB yaclLibSpace::yaclLib* ENGINE;  
#define CMDS_LIST yaclLibSpace::cmdLineCommand
#define INIT_CMDS(s, c) cmdEngine = new yaclLibSpace::yaclLib(s, c, NELEMS(c));
#define CHECK_CMDS cmdEngine->checkCommands();
#define CMD_GETINT ENGINE->readNumber()  
#define CMD_GETLONG ENGINE->readLong()  
#define CMD_GETSTR ENGINE->readWord()  
#define CMD_OK ENGINE->readOk()

namespace yaclLibSpace {

const unsigned int CR = '\r';
const unsigned int LF = '\n';
const unsigned int BS = '\b';
const unsigned int NULLCHAR = '\0';
const unsigned int SPACE = ' ';
const unsigned int COMMAND_BUFFER_LENGTH = 32;

typedef void (*num_func)();
extern num_func functions[];

typedef struct cmdLineCommand {
    char* tokens;
    num_func userFunc;
} cmdLineCommand;

// Generic template
template<class T> 
inline Print &operator <<(Print &stream, T arg) 
{ stream.print(arg); return stream; }

class yaclLib : public Stream {
  public:
//    yaclLib(Stream& dev, num_func* func, char** toks, size_t s);
    yaclLib(Stream& dev, cmdLineCommand* cmds, size_t s);
   ~yaclLib();
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
//    num_func* userFunc;
//    char** tokens;
    cmdLineCommand* commands;
//    cmdLineCommand* cmdLineCommands;
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

} /* namespace yaclLibSpace */
#endif /* YACLLIB_H_ */
