/*
 * cmdLineLib.cpp
 *
 *  Created on: 11 03 2019
 *      Author: Cricri042
 */

#include "cmdLineLib.h"

namespace CmdLineLibSpace {

///************************************************/
//cmdLineLib::cmdLineLib(Stream& dev, num_func* func, char** toks, size_t s) 
//  : stream(dev), userFunc(func), userFuncSize(s), tokens(toks) {
//  ok = true;
//}
/************************************************/
cmdLineLib::cmdLineLib(Stream& dev, cmdLineCommand* cmds, size_t s) 
  : stream(dev), commands(cmds), userFuncSize(s) {
  ok = true;
}

/************************************************/
cmdLineLib::~cmdLineLib() {
}

/************************************************/
int cmdLineLib::read() {
  return stream.read();
}
/************************************************/
int cmdLineLib::available() {
  return stream.available();
}
/************************************************/
int cmdLineLib::peek() {
  return stream.peek();
}
/************************************************/
size_t cmdLineLib::write(uint8_t b) {
  return stream.write(b);
}

/*************************************************************************************************************
 getCommandLineFromSerialPort()
 Return the string of the next command. Commands are delimited by return"
 Handle BackSpace character
 Make all chars lowercase
 *************************************************************************************************************/
bool cmdLineLib::getCommandLineFromSerialPort() {
	//    static uint8_t charsRead = 0;                      //note: COMAND_BUFFER_LENGTH must be less than 255 chars long
	//    Serial.println("CHECK");
	//read asynchronously until full command input
	while (stream.available()) {
		char c = stream.read();
		switch (c) {
		case CR: //likely have full command in buffer now, commands are terminated by CR and/or LS
		case LF:
			commandLine[charsRead] = NULLCHAR; //null terminate our command char array
			if (charsRead > 0) {
				charsRead = 0; //charsRead is static, so have to reset
				//stream.println(commandLine);
				return true;
			}
			break;
		case BS: // handle backspace in input: put a space in last char
			if (charsRead > 0)
			{ //and adjust commandLine and charsRead
				commandLine[--charsRead] = NULLCHAR;
				stream << byte(BS) << byte(SPACE) << byte(BS); //no idea how this works, found it on the Internet
//          stream.print(BS);
//          stream.print(SPACE);
//          stream.print(BS);
			}
			break;
		default:
			// c = tolower(c);
			if (charsRead < COMMAND_BUFFER_LENGTH) {
				commandLine[charsRead++] = c;
			}
			commandLine[charsRead] = NULLCHAR; //just in case
			break;
		}
	}
	return false;
}

/* ****************************
 readNumber: return a 16bit (for Arduino Uno) signed integer from the command line
 readWord: get a text word from the command line
 */
int cmdLineLib::readNumber() {
	ok = true;
	char *numTextPtr = strtok(NULL, delimiters); //K&R string.h  pg. 250
	if (numTextPtr == NULL) {
		ok = false;
	}
	return atoi(numTextPtr); //K&R string.h  pg. 251
}

/************************************************/
long cmdLineLib::readLong() {
	ok = true;
	char *numTextPtr = strtok(NULL, delimiters); //K&R string.h  pg. 250
	if (numTextPtr == NULL) {
		ok = false;
	}
	return atol(numTextPtr); //K&R string.h  pg. 251
}

/************************************************/
char *cmdLineLib::readWord() {
	char *word = strtok(NULL, delimiters); //K&R string.h  pg. 250
	return word;
}

/************************************************/
void cmdLineLib::nullCommand(char *ptrToCommandName) {
	print2(F("Command not found: "), ptrToCommandName); //see above for macro print2
}

/************************************************/
int cmdLineLib::isNumericString(char *s) {
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
void cmdLineLib::DoMyCommand() {
	print2("\nCommand: ", commandLine);  
	char *ptrToCommandName = strtok(commandLine, delimiters);
	//  print2("commandName= ", ptrToCommandName);

  for (int i = 0; i < userFuncSize; ++i) {
//      if (strcmp(ptrToCommandName, tokens[i]) == 0) {
      if (strcmp(ptrToCommandName, commands[i].tokens) == 0) {
        
//            stream.println("OK");
            commands[i].userFunc();
            return;
      }
  }
   nullCommand(ptrToCommandName);
}

/************************************************/
void cmdLineLib::checkCommands() {
	noInterrupts();
	// disable all interrupts
	bool received = getCommandLineFromSerialPort();
	if (received)	DoMyCommand();
	interrupts();
	// enable all interrupts
}

} /* namespace CmdLineLibSpace */
