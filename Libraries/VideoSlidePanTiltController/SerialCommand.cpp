/******************************************************************************* 
SerialCommand - An Arduino library to tokenize and parse commands received over
a serial port. 
Copyright (C) 2011-2013 Steven Cogswell  <steven.cogswell@gmail.com>
http://awtfy.com

See SerialCommand.h for version history. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************************/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "SerialCommand.h"

#include <string.h>


#ifdef SERIALCOMMAND_HARDWAREONLY

SerialCommand::SerialCommand() 
{
  strncpy(delim," ", sizeof(delim));  // strtok_r needs a null-terminated string
  clearBuffer(); 
}

#else

SerialCommand::SerialCommand(SoftwareSerial *softSer) : 
  softSerial(softSer)
{		
  strncpy(delim," ", sizeof(delim));  // strtok_r needs a null-terminated string
  clearBuffer(); 
}


#endif



/**
 * Initialize the command buffer being processed to all null characters
 */
void SerialCommand::clearBuffer()
{
  // for (int i = 0; i < sizeof(buffer); i++) buffer[i]='\0';
  buffer[0] = '\0';
  bufPos=0; 
}

/**
 * Retrieve the next token ("word" or "argument") from the Command buffer.  
 * returns a NULL if no more tokens exist.   
 */
char *SerialCommand::next() 
{
  return strtok_r(NULL, delim, &last); 
}

bool SerialCommand::available()
{
#ifdef SERIALCOMMAND_HARDWAREONLY
  return (Serial.available() > 0);
#else
  return (((softSerial != 0) ? softSerial->available() : Serial.available()) > 0) 
#endif
}

/** 
 * Check the Serial stream for characters, and assemble them into a buffer.  
 * When the terminator character (default '\r') is seen, 
 * pars the buffer for a prefix command, and calls handlers setup in addCommand()
 */
void SerialCommand::readAndDispatch() 
{
  // If we're using the Hardware port, check it.   Otherwise check the user-created SoftwareSerial Port
  while(available()) 
  {
    char inChar = 
#ifdef SERIALCOMMAND_HARDWAREONLY
      Serial.read();
#else
      (softSerial != 0) ? softSerial->read() : Serial.read();
#endif

#ifdef SERIALCOMMANDDEBUG
      Serial.print(inChar);   // Echo back to serial stream
#endif
		if(isprint(inChar))   // Only printable characters into the buffer
		{
                  buffer[bufPos++] = toupper(inChar);   // Put character into buffer
                  buffer[bufPos]='\0';  // Null terminate
                  if(bufPos >= (sizeof(buffer)-1))
                    bufPos = 0; // wrap buffer around if full  
		}
		else if (inChar == term) // Check for the terminator (default '\r') meaning end of command
                {     
                        //process the command                  
#ifdef SERIALCOMMANDDEBUG
			Serial.print("Received: "); 
			Serial.println(buffer);
#endif
			bufPos = 0;           // Reset to start of buffer
			char *token = strtok_r(buffer, delim, &last);   // Search for command at start of buffer
			if (token == NULL) return; 
                        boolean matched = false;			
			for(int i = 0; i < numCommand; i++) {
#ifdef SERIALCOMMANDDEBUG
				Serial.print("Comparing ["); 
				Serial.print(token); 
				Serial.print("] to [");
				Serial.print(CommandList[i].command);
				Serial.println("]");
#endif
				// Compare the found command against the list of known commands for a match
				if (strncmp(token, commandList[i].command, SERIALCOMMANDBUFFER) == 0) 
				{
#ifdef SERIALCOMMANDDEBUG
					Serial.print("Matched Command: "); 
					Serial.println(token);
#endif
					// Execute the stored handler function for the command
					(*commandList[i].function)(); 
					matched=true; 
					break; 
				}
			}
			if (!matched) {
				(*defaultHandler)(); 
			}
			clearBuffer(); 
		}
	}
}

/**
 * Adds a "command" and a handler function to the list of available commands.  
 * This is used for matching a found token in the buffer, and gives the pointer
 * to the handler function to deal with it. 
 */
bool SerialCommand::addCommand(const char *command, void (*function)())
{
  if (numCommand >= MAXSERIALCOMMANDS) {
    // you tried to push more commands into the buffer than it is compiled to hold.  
    // Not much we can do since there is no real visible error assertion, 
    // we just ignore adding the command
#ifdef SERIALCOMMANDDEBUG
    Serial.println("Too many handlers - recompile changing MAXSERIALCOMMANDS"); 
#endif 
    return false;
  }
#ifdef SERIALCOMMANDDEBUG
  Serial.print(numCommand); 
  Serial.print("-"); 
  Serial.print("Adding command for "); 
  Serial.println(command); 
#endif		
  commandList[numCommand].command = command;
  commandList[numCommand].function = function; 
  numCommand++; 
  return true;
}




