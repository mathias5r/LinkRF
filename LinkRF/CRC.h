/*
 * CRC.h
 *
 *  Created on: 13 de out de 2016
 *      Author: mathias
 */

#ifndef CRC_H_
#define CRC_H_

#include <iostream>
#include <cstring>
#include <stdint.h>

using namespace std;

#define BUFSIZE 1024

#define PPPINITFCS16 0xffff
#define PPPGOODFSCS16 0xf0b8

class CRC {
public:
	CRC(){};
	virtual ~CRC(){};
	bool check_crc(unsigned char * buffer, int len);
	void gen_crc(unsigned char * buffer, int len);
	uint16_t pppfcs16(unsigned char * cp, int len);
};

#endif /* CRC_H_ */
