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
	this->canSend = true;
}

void ARQ::type(){};

bool ARQ::handle(char * buffer, int len){
	int fd = 0; // o descritor 0 corresponde à entrada padrão ...
	int max_fd = fd;
	struct timeval timeout; // para especificar o timeout
	timeout.tv_sec = 5; //timeout de 2 segundos
	timeout.tv_usec = 0;
	fd_set espera; // um conjunto de descritores
	FD_ZERO(&espera); // zera o conjunto de descritores
	FD_SET(fd, &espera); // adiciona "fd" ao conjunto de descritores

	switch(this->currentstate){
		case A:
			if (select(max_fd+1, &espera, NULL, NULL, &timeout) == 0) {
			    puts("Timeout !");
			}else {
			    // a seguir se verifica que descritores podem ser lidos sem risco de bloqueio
			    // i.e.: que descritores estão prontos para serem acessados
			    if (FD_ISSET(fd, &espera)) {
					this->framework.send(buffer,len,0,sequenceN); // Como é feito o número de sequência?
					this->currentstate = B;
			    }else if(this->framework.get_bytes() > 0){ // Essa função não pode ser bloqueiante, como fazer?
					this->framework.send((char *)"",0,1,sequenceM);// Como é feito o número de sequência?
					this->currentstate = A;
			    }
			}
//
//			if(Event(RequestToSend)){ // Como saber se é payload?
//				this->framework.send(buffer,len,0,sequenceN); // Como é feito o número de sequência?
//				this->currentstate = B;
//			}else if(receive(dataM) == true){ // Essa função não pode ser bloqueiante, como fazer?
//				this->framework.send((char *)"",0,1,sequenceM);// Como é feito o número de sequência?
//				this->currentstate = A;
//			}
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


