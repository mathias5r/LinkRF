/*
 * ARQ.h
 *
 *  Created on: 20 de out de 2016
 *      Author: mathias
 */

#include "Framework.h"

#ifndef ARQ_H_
#define ARQ_H_

using namespace std;


class ARQ {
public:
	ARQ(Framework * f, int transceiver, int aplicacao);
	virtual ~ARQ(){};
	bool send(char * buffer, int len);
	bool test_data(Framework::Type);
	bool test_ack(Framework::Type);

	void set_canSend(bool state){ this->canSend = state; };
	void set_received(bool state){ this->received = state; };
	void set_timeout(bool state){ this->timeout = state; };
	void set_backoff(bool state){ this->backoff = state; };

	bool get_canSend(){ return this->canSend; };
	bool get_received(){ return this->received; };
	bool get_timeout(){ return this->timeout; };
	bool get_backoff(){ return this->backoff; };

	bool handle();

private:
	Framework * framework;
	enum State{
		A,B,C,D
	};
	State currentstate;
	int sequenceN;
	int sequenceM;
	bool canSend, received, timeout, backoff;
	int transceiver, aplicacao;
};

#endif /* ARQ_H_ */
