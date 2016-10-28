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
	this->canSend = false;
	this->received = false;
	this->backoff = false;
	this->timeout = false;
}

bool ARQ::handle(char * buffer, int len){

	switch(this->currentstate){
		case A:
			if(this->canSend){
				this->framework.send(buffer,len,0,sequenceN);
				this->currentstate = B;
			}else if(this->received){
				test_data(buffer,len);
				this->currentstate = A;
			}
			break;
		case B:
			if(this->received){
				if(test_data(buffer,len)){
					int rd = rand() % 10 + 1;
					usleep(rd*1000); //BACKOFF
					this->currentstate = B;
				}else if(test_ack(buffer,len)){
					int rd = rand() % 10 + 1;
					usleep(rd*1000); //BACKOFF
					this->currentstate = C;
				}
			}else if(this->timeout){
				int rd = rand() % 10 + 1;
				usleep(rd*1000); //BACKOFF
				this->currentstate = D;
			}
			break;
		case C:
			if(this->backoff){
				this->currentstate = A;
			}else if(this->received){
				test_data(buffer,len);
				this->currentstate = C;
			}
			break;
		case D:
			if(this->backoff){
				if(!(this->framework.send(buffer,len,0,sequenceN)) > 0){
					cout << "Erro ao enviar quadro de sequencia: " << this->sequenceN << endl;
				}
				this->currentstate = B;
			}else if(this->received){
				if(test_data(buffer,len)){
					this->currentstate = D;
				}
			}
			break;
	}
	return false;
}

bool ARQ::test_data(char * buffer, int len){

	Type r = get_type(buffer,len);
	if(r == data0){
		this->sequenceM = 0;
		if(!(this->framework.send((char*)"",0,1,this->sequenceM) > 0)){
			cout << "Erro ao enviar quadro de sequencia: " << this->sequenceM;
		}
		return true;
	}else if(r == data1){
		this->sequenceM = 1;
		if(!(this->framework.send((char*)"",0,1,this->sequenceM) > 0)){
			cout << "Erro ao enviar quadro de sequencia: " << this->sequenceM;
		}
		return true;
	}else{
		cout << "Erro: tipo de mensagem não coerente" << endl;
	}
	return false;
}

bool ARQ::test_ack(char * buffer, int len){

	Type r = get_type(buffer,len);
	if(r == ack0){
		if(sequenceN == 0){
			sequenceN = 1;
			//CONFIGURAR BACKOFF E INICIÁ-LO
			return true;
		}
	}else if( r == ack1){
		if(sequenceN == 1){
			sequenceN = 0;
			//CONFIGURAR BACKOFF E INICIÁ-LO
			return true;
		}
	}

	return false;
}

ARQ::Type ARQ::get_type(char * buffer, int len){

	switch(buffer[1]){
		case(0x00):
			return data0;
			break;
		case(0x01):
			return data1;
			break;
		case(0x02):
			return ack0;
			break;
		case(0x03):
			return ack1;
			break;
		default:
			cout << "Erro: Tipo de mensagem não identificada!: " << buffer[1] << endl;
			break;
	}
	return none;
}

