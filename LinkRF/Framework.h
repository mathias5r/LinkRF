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
#include <stdint.h>
#include<cstdlib>

#ifndef FRAMMING_H_
#define FRAMMING_H_

#define PPPINITFCS16 0xffff
#define PPPGOODFSCS16 0xf0b8

#define BUFSIZE 1024

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

	// verifica o CRC do conteúdo contido em "buffer". Os dois últimos
	// bytes desse buffer contém o valor de CRC
	bool check_crc(unsigned char * buffer, int len);

	// gera o valor de CRC dos bytes contidos em buffer. O valor de CRC
	// é escrito em buffer[len] e buffer[len+1]
	void gen_crc(unsigned char * buffer, int len);

	// calcula o valor de CRC dos bytes contidos em "cp".
	// "fcs" deve ter o valor PPPINITFCS16
	// O resultado é o valor de CRC (16 bits)
	// OBS: adaptado da RFC 1662 (enquadramento no PPP)
	uint16_t pppfcs16(unsigned char * cp, int len);

};

#endif /* FRAMMING_H_ */
