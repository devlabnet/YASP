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
#define YACL_USE_YACLLIB yaclLibSpace::yaclLib* ENGINE;
#define YACL_CMDS_LIST yaclLibSpace::cmdLineCommand
#define YACL_INIT_CMDS(s, c) cmdEngine = new yaclLibSpace::yaclLib(s, c, NELEMS(c));
#define YACL_CHECK_CMDS cmdEngine->checkCommands();
#define YACL_GETINT ENGINE->readNumber()
#define YACL_GETLONG ENGINE->readLong()
#define YACL_GETFLOAT ENGINE->readFloat()
#define YACL_GETSTR ENGINE->readWord()
#define YACL_OK ENGINE->readOk()
#define YACL_PRINT(x) ENGINE->stream.print(x)
#define YACL_PRINTLN(x) ENGINE->stream.println(x)
#define YACL_PRINT2(x, y) ENGINE->stream.print(x, y)
#define YACL_PRINTLN2(x, y) ENGINE->stream.println(x, y)
#define YACL_WRITE(x) ENGINE->stream.write(x)
#define YACL_TOKEN ENGINE->token()

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
    yaclLib(Stream& dev, cmdLineCommand* cmds, size_t s);
    ~yaclLib();
    Stream &stream;
    /***************************************************************************
        getCommandLineFromSerialPort()
          Return the string of the next command. Commands are delimited by return"
          Handle BackSpace character
          Make all chars lowercase
    ****************************************************************************/
    bool getCommandLineFromSerialPort();
    int readNumber();
    long readLong();
    float readFloat();
    char* readWord();
    char* token() { return cmdToken; };
    void checkCommands();
    bool readOk() { return ok; }
    virtual void flush() { /* Empty implementation for backward compatibility */ }

private:
    int read();
    int available();
    int peek();
    size_t write(uint8_t b);
    cmdLineCommand* commands;
    size_t userFuncSize;
    char commandLine[COMMAND_BUFFER_LENGTH + 1]; //Read commands into this buffer from Serial.  +1 in length for a termination char
    bool ok;
    const char *delimiters = " "; //commands must be separated by space
    char *cmdToken = "";
    int isNumericString(char *s);
    void nullCommand(char *ptrToCommandName);
    void DoMyCommand();
};

} /* namespace yaclLibSpace */
#endif /* YACLLIB_H_ */
