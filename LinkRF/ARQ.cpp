/*
 * ARQ.cpp
 *
 *  Created on: 20 de out de 2016
 *      Author: mathias
 */

#include "ARQ.h"

ARQ::ARQ(Framework & f):framework(f){
	this->currentstate = A;
	this->sequenceN = 0;
	this->sequenceM = 0;
}

void ARQ::type(){};

bool ARQ::handle(char * buffer, int len){

	switch(this->currentstate){
		case A:
			if(payload == true){ // Como saber se é payload?
				this->framework.send(buffer,len,0,sequenceN); // Como é feito o número de sequência?
				this->currentstate = B;
			}else if(receive(dataM) == true){ // Essa função não pode ser bloqueiante, como fazer?
				this->framework.send((char *)"",0,1,sequenceM);// Como é feito o número de sequência?
				this->currentstate = A;
			}
			break;
		case B:
			if(receive(ackN) == true){
				set_backoff();
				this->currentstate = C;
			}else if(receive(dataM) == true){
				this->framework.send((char *)"",0,1,sequenceM);
				this->currentstate = B;
			}else if(timeout == true || receive(ackN)){
				set_backoff();
				this->currentstate = D;
			}
			break;
		case C:
			if(backoff == true){
				this->currentstate = A;
			}else if(receive(dataM)){
				this->framework.send((char *)"",0,1,sequenceM);
				this->currentstate = C;
			}
			break;
		case D:
			if(backoff == true){
				this->framework.send(buffer,len,0,sequenceN);
				this->currentstate = B;
			}else if(receive(dataM)){
				this->framework.send((char *)"",0,1,sequenceM);
				this->currentstate = D;
			}
			break;
	}
	return false;
}


