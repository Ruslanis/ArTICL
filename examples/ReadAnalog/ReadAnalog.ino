/*************************************************
 *  ReadAnalog.ino                               *
 *  Example from the ArTICL library              *
 *           Created by Christopher Mitchell,    *
 *           2011-2014, all rights reserved.     *
 *                                               *
 *  This demo reads the Arduino's six analog     *
 *  pins whenever the calculator requests a      *
 *  list, and returns the results as a six-      *
 *  element list with values between 0 and 1023. *
 *************************************************/

#include "CBL2.h"
#include "TIVar.h"

CBL2* cbl;
int lineRed = 7;
int lineWhite = 6;

#define MAXDATALEN 255
uint8_t header[16];
uint8_t data[MAXDATALEN];

void setup() {
  Serial.begin(9600);
  cbl = new CBL2(lineRed, lineWhite);
  cbl->resetLines();
  cbl->setVerbosity(true, &Serial);			// Comment this in for mesage information
  cbl->setupCallbacks(header, data, MAXDATALEN,
                      onGetAsCBL2, onSendAsCBL2);
}

void loop() {
  int rval;
  rval = cbl->eventLoopTick();
  if (rval && rval != ERR_READ_TIMEOUT) {
    Serial.print("Failed to run eventLoopTick: code ");
    Serial.println(rval);
  }
}

int onGetAsCBL2(uint8_t type, enum Endpoint model, int datalen) {
  Serial.print("Got variable of type ");
  Serial.print(type);
  Serial.print(" from endpoint of type ");
  Serial.println((int)model);
  return 0;
}

int onSendAsCBL2(uint8_t type, enum Endpoint model, int* headerlen,
                 int* datalen, data_callback* data_callback)
{
  Serial.print("Got request for variable of type ");
  Serial.print(type);
  Serial.print(" from endpoint of type ");
  Serial.println((int)model);
  
  if (type != 0x01)
    return -1;
  
  // Compose the VAR header
  *datalen = 2 + TIVar::sizeOfReal(model) * 6;
  TIVar::intToSizeWord(*datalen, &header[0]);	// Two bytes for the element count, 6 Reals
  header[1] = 0;
  header[2] = 0x04;
  header[3] = 0x01;
  header[4] = 0x41;				// See http://www.cemetech.net/forum/viewtopic.php?p=224739#224739
  header[5] = 0x00;
  *headerlen = 11;
  
  // Compose the body of the variable
  data[0] = 6;
  data[1] = 0;
  int offset = 2;
  for(int i = 0; i < 6; i++) {
	long value = analogRead(i);
	// Convert the value, get the length of the inserted data or -1 for failure
	int rval = TIVar::longToReal8x(value, &data[offset], model);
	if (rval < 0) {
		return -1;
	}
	offset += rval;
  }
  for(int i = 0; i < *datalen; i++) {
    Serial.print(data[i], HEX);
	Serial.print(" ");
  }
  Serial.println("]]");

  return 0;
}