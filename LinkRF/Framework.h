/*
 * Framming.h

 *
 *  Created on: 25 de ago de 2016
 *      Author: aluno
 */
#include "Serial.h"
#include <string.h>
#include <iostream>
#include <cstring>

#ifndef FRAMMING_H_
#define FRAMMING_H_

class Framework {

public:
	Framework(Serial& s, int bytes_min, int bytes_max);
	virtual ~Framework();
	void send(char *buffer, int len);
	int receive(char* buffer);

	// getters
	Serial& get_serial() { return serial; }
	int get_max_bytes() { return max_bytes; }
	int get_min_bytes() { return min_bytes; }
	char * get_buffer() { return buffer; }

	// setters
	void set_buffer(char * buff) {buffer = buff; }
	void set_serial(Serial & __serial) {serial = __serial; }
	void set_min_bytes(int bytes_min) {min_bytes = bytes_min;}
	void set_max_bytes(int bytes_max) {max_bytes = bytes_max;}

private:
	Serial & serial;
	int min_bytes, max_bytes; // max and min number of bytes allowed for each frame
	char * buffer; // should be dimensioned instantiation

	// bytes recebidos pela MEF até o momento
	int n_bytes;

	// estados para FSM
	enum State {
		waiting,reception,escape
	};

	State currentState;

	// aqui se implementa a máquina de estados de recepção
	// retorna true se reconheceu um quadro completo
	bool handle(char byte);


};

#endif /* FRAMMING_H_ */
