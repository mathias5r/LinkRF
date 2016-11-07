/*
 * Dispacher.h
 *
 *  Created on: 27 de out de 2016
 *      Author: mathias
 */

#ifndef DISPACHER_H_
#define DISPACHER_H_

#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

#include "Tun.h"
#include "ARQ.h"
#include "Framework.h"
#include "Serial.h"

using namespace std;

class Dispacher {
public:
	Dispacher(ARQ & a, int fd_transceiver, int fd_serial);
	Dispacher(const Dispacher& d);
	void handle();
	void handle_forever();
	virtual ~Dispacher();
private:
	ARQ & arq;
	int transceiver, aplicacao;

};

#endif /* DISPACHER_H_ */
