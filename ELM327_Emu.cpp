/*
 *  ELM327_Emu.cpp
 *
 * Class emulates the serial comm of an ELM327 chip - Used to create an OBDII interface
 *
 * Created: 3/18/2014
 *  Author: Collin Kidder
 */

/*
 Copyright (c) 2013-2014 Collin Kidder, Michael Neuweiler, Charles Galpin

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ELM327_Emu.h"

/*
 * Initialization of hardware and parameters
 */
void ICHIPWIFI::setup() {

	prefsHandler = new PrefHandler(ELM327EMU);

	TickHandler::getInstance()->detach(this);

	tickCounter = 0;
	ibWritePtr = 0;
	serialInterface->begin(9600);

	//this isn't a wifi link but the timer interval can be the same
	//because it serves a similar function and has similar timing requirements
	TickHandler::getInstance()->attach(this, CFG_TICK_INTERVAL_WIFI);
}

/*
 * Send a command to ichip. The "AT+i" part will be added.
 */
void ELM327Emu::sendCmd(String cmd) {
	serialInterface->write("AT");
	serialInterface->print(cmd);
	serialInterface->write(13);
	loop(); // parse the response
}

/*
 * Periodic updates of parameters to ichip RAM.
 * Also query for changed parameters of the config page.
 */
void ELM327Emu::handleTick() {

}

/*
 * Handle a message sent by the DeviceManager.
 * Currently MSG_SET_PARAM is supported. A array of two char * has to be included
 * in the message.
 */
void ELM327Emu::handleMessage(uint32_t messageType, void* message) {
	Device::handleMessage(messageType, message);

	switch (messageType) {
	case MSG_SET_PARAM: {
		break;
	}
	case MSG_CONFIG_CHANGE:
		break;
	case MSG_COMMAND:
		sendCmd((char *)message);
		break;
	}
}

/*
 * Constructor. Assign serial interface to use for comm with bluetooth adapter we're emulating with
 */
ELM327Emu::ELM327Emu() {
	prefsHandler = new PrefHandler(ELM327EMU);

	uint8_t sys_type;
	sysPrefs->read(EESYS_SYSTEM_TYPE, &sys_type);
	if (sys_type == 3 || sys_type == 4)
		serialInterface = &Serial2;
	else //older hardware used this instead
		serialInterface = &Serial3; 
}

/*
 * Constructor. Pass serial interface to use
 */
ELM327Emu::ELM327Emu(USARTClass *which) {
	prefsHandler = new PrefHandler(ELM327EMU);
	serialInterface = which;
}

/*
 * Called in the main loop (hopefully) in order to process serial input waiting for us
 * from the wifi module. It should always terminate its answers with 13 so buffer
 * until we get 13 (CR) and then process it.
 * But, for now just echo stuff to our serial port for debugging
 */

void ELM327Emu::loop() {
	int incoming;
	while (serialInterface->available()) {
		incoming = serialInterface->read();
		if (incoming != -1) { //and there is no reason it should be -1
			if (incoming == 13 || ibWritePtr > 126) { // on CR or full buffer, process the line
				incomingBuffer[ibWritePtr] = 0; //null terminate the string
				ibWritePtr = 0; //reset the write pointer
				/*
				if (strchr(incomingBuffer, '=') && (strncmp(incomingBuffer, Constants::ichipCommandPrefix, 4) != 0))
					processParameterChange(incomingBuffer);
				else if (strchr(incomingBuffer, ','))
					Logger::info(ICHIP2128, incomingBuffer);
				else if (Logger::isDebug())
					Logger::debug(ICHIP2128, incomingBuffer);
					*/
			} else { // add more characters
				if (incoming != 10) // don't add a LF character
					incomingBuffer[ibWritePtr++] = (char) incoming;
			}
		} else
			return;
	}
}

DeviceType ELM327Emu::getType() {
	return DEVICE_MISC;
}

DeviceId ELM327Emu::getId() {
	return (ELM327EMU);
}

void ELM327Emu::loadConfiguration() {
	ELM327Configuration *config = new ELM327Configuration();

	if (prefsHandler->checksumValid()) { //checksum is good, read in the values stored in EEPROM
		Logger::debug(ELM327EMU, "Valid checksum so using stored elm327 emulator config values");
		//TODO: implement processing of config params for WIFI
//		prefsHandler->read(EESYS_WIFI0_SSID, &config->ssid);
	}
}

void ELM327Emu::saveConfiguration() {
	ELM327Configuration *config = (ELM327Configuration *) getConfiguration();

	//TODO: implement processing of config params for WIFI
//	prefsHandler->write(EESYS_WIFI0_SSID, config->ssid);
//	prefsHandler->saveChecksum();
}
