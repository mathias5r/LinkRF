/*
 * Framming.h

 *
 *  Created on: 25 de ago de 2016
 *      Author: aluno
 */
#include "Serial.h"
#include "CRC.h"
#include <string.h>
#include <iostream>
#include <cstring>
#include <stdint.h>
#include <cstdlib>

#ifndef FRAMMING_H_
#define FRAMMING_H_

#define PPPINITFCS16 0xffff
#define PPPGOODFSCS16 0xf0b8

using namespace std;

#define BUFSIZE 1024
#define FRAME_MAXSIZE 2*BUFSIZE+5
#define FRAME_MINSIZE 6

class Framework{

public:
	Framework(Serial& s, int bytes_min, int bytes_max);
	virtual ~Framework(){};
	int send(char *buffer, int len, int type, int seq);
	int receive(char* buffer);
	char * mount(char* data, int len, int type, int seq);

	// getters
	Serial& get_serial() { return serial; }
	int get_max_bytes() { return max_bytes; }
	int get_min_bytes() { return min_bytes; }
	char * get_buffer() { return buffer; }
	int get_bytes() { return n_bytes; }

	// setters
	void set_buffer(char * buff) { buffer = buff; }
	void set_serial(Serial & __serial) { serial = __serial; }
	void set_min_bytes(int bytes_min) { min_bytes = bytes_min;}
	void set_max_bytes(int bytes_max) { max_bytes = bytes_max;}

private:

	CRC * crc;
	Serial & serial;
	int min_bytes, max_bytes; // max and min number of bytes allowed for each frame
	char * buffer;

	// bytes recebidos pela MEF até o momento
	int n_bytes;

	enum State {
		waiting,reception,escape
	};

	State currentState;

	bool handle(char byte);

	char header(int type, int seq);

};

#endif /* FRAMMING_H_ */
