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
	ARQ(Framework & f);
	virtual ~ARQ(){};
	bool send(char * buffer, int len);
private:
	void type();
	Framework & framework;
	enum State{
		A,B,C,D
	};
	State currentstate;
	int sequenceN, sequenceM
	bool canSend;
	bool handle(char * buffer, int len);
};

#endif /* ARQ_H_ */
