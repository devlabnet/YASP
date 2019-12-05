/*
   yaclLib.cpp

    Created on: 11 03 2019
        Author: Cricri042
*/
#include "yaclLib.h"

namespace yaclLibSpace {
/************************************************/
yaclLib::yaclLib(Stream& dev, cmdLineCommand* cmds, size_t s)
  : stream(dev), commands(cmds), userFuncSize(s) {
  ok = true;
  stream.setTimeout(10);
}
/************************************************/
yaclLib::~yaclLib() {
}
/************************************************/
int yaclLib::read() {
  return stream.read();
}
/************************************************/
int yaclLib::available() {
  return stream.available();
}
/************************************************/
int yaclLib::peek() {
  return stream.peek();
}
/************************************************/
size_t yaclLib::write(uint8_t b) {
  return stream.write(b);
}

/*************************************************************************************************************
  getCommandLineFromSerialPort()
  Return the string of the next command. Commands are delimited by return"
  Handle BackSpace character
  Make all chars lowercase
 *************************************************************************************************************/
bool yaclLib::getCommandLineFromSerialPort() {
  if (stream.available()) {
    int cr = readBytesUntil('\n', commandLine, COMMAND_BUFFER_LENGTH);
    int l = strcspn(commandLine, "\r\n");
    commandLine[l] = NULLCHAR;
    return (1 > 0);
  }
  return false;
}

/* ****************************
  readNumber: return a 16bit (for Arduino Uno) signed integer from the command line
  readWord: get a text word from the command line
*/
int yaclLib::readNumber() {
  ok = true;
  char *numTextPtr = strtok(NULL, delimiters); //K&R string.h  pg. 250
  if (numTextPtr == NULL) {
    ok = false;
  }
  return atoi(numTextPtr); //K&R string.h  pg. 251
}

/************************************************/
long yaclLib::readLong() {
  ok = true;
  char *numTextPtr = strtok(NULL, delimiters); //K&R string.h  pg. 250
  if (numTextPtr == NULL) {
    ok = false;
  }
  return atol(numTextPtr); //K&R string.h  pg. 251
}

/************************************************/
float yaclLib::readFloat() {
  ok = true;
  char *numTextPtr = strtok(NULL, delimiters); //K&R string.h  pg. 250
  if (numTextPtr == NULL) {
    ok = false;
  }
  return atof(numTextPtr); //K&R string.h  pg. 251
}

/************************************************/
char *yaclLib::readWord() {
  char *word = strtok(NULL, delimiters); //K&R string.h  pg. 250
  return word;
}

/************************************************/
void yaclLib::nullCommand(char *ptrToCommandName) {
  print2(F("Command not found: "), ptrToCommandName); //see above for macro print2
}

/************************************************/
int yaclLib::isNumericString(char *s) {
  int i = 0, isNumeric = 1, ctDecimalPointsSeen = 0;
  if (s[i] == '+' || s[i] == '-')	{
    i += 1;
  } //Start from the next character
  while (s[i] != '\0') {
    if (!isDigit(s[i]))	{
      if (s[i] == '.' && ctDecimalPointsSeen < 1)	{
        ctDecimalPointsSeen += 1;
      }	else { /*if the number of decimal points is more than 1 or if the character encountered isn't a digit or .*/
        isNumeric = 0;
        break;
      }
    }
    i += 1;
  }
  return isNumeric;
}

/****************************************************
  DoMyCommand
*/
void yaclLib::DoMyCommand() {
  if (strlen(commandLine) == 0) return;
  //print2("\nCommand: ", commandLine);
  char* ptrToCommandName = strtok(commandLine, delimiters);
  for (int i = 0; i < userFuncSize; ++i) {
    if (strcmp(ptrToCommandName, commands[i].tokens) == 0) {
      commands[i].userFunc();
      commandLine[0] = NULLCHAR;
      return;
    }
  }
  //    cmdToken = "";
  nullCommand(ptrToCommandName);
  commandLine[0] = NULLCHAR;
}

/************************************************/
void yaclLib::checkCommands() {
  // disable all interrupts
  //noInterrupts();
  bool received = getCommandLineFromSerialPort();
  if (received)	DoMyCommand();
  // enable all interrupts
  //interrupts();
}

} /* namespace yaclLibSpace */
