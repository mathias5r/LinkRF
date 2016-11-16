/*
 * ARQ.h
 *
 *  Created on: 20 de out de 2016
 *      Author: mathias
 */

#include "Framework.h"
#include "Tun.h"
#include "Frame.h"
#include <cstdlib>
#include <ctime>

#ifndef ARQ_H_
#define ARQ_H_

using namespace std;

class ARQ {
public:
	ARQ(Framework & f, Tun & tun);
	virtual ~ARQ(){};
	bool send(char * buffer, int len);
	bool test_data(Framework::Type);
	bool test_ack(Framework::Type);

	void set_canSend(bool state){ this->canSend = state; };
	void set_received(bool state){ this->received = state; };
	void set_timeout(bool state){ this->timeout = state; };
	void set_backoff(bool state){ this->backoff = state; };
	void set_enable(bool state){ this->enable = state; };

	bool get_canSend(){ return this->canSend; };
	bool get_received(){ return this->received; };
	bool get_timeout(){ return this->timeout; };
	int get_backoff(){ return this->backoff_value; };
	int get_enable(){ return this->enable; };

	bool handle();

private:
	Framework & framework;
	Tun & tun;
	Frame current_frame;
	enum State{
		A,B,C,D
	};
	State currentstate;
	int sequenceN;
	int sequenceM;
	bool canSend, received, timeout, backoff, enable;
	int backoff_value;
};

#endif /* ARQ_H_ */
