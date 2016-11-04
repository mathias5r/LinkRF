/*
 * ARQ.cpp
 *
 *  Created on: 20 de out de 2016
 *      Author: mathias
 */

#include "ARQ.h"

ARQ::ARQ(Framework * f, int transceiver, int aplicacao){
	this->currentstate = A;
	this->sequenceN = 0;
	this->sequenceM = 0;
	this->canSend = false;
	this->received = false;
	this->backoff = false;
	this->timeout = false;
	this->aplicacao = aplicacao;
	this->transceiver = transceiver;
	this->framework = f;
}

bool ARQ::handle(){

	switch(this->currentstate){
		case A:
			cout << "Estado A " << endl;
			if(this->canSend){ // Recebeu um payload da aplicação
				this->framework->send(0,this->sequenceN);
				this->currentstate = B;
			}else if(this->received){
				Framework::Type r = this->framework->receive();
				test_data(r);
				this->currentstate = A;
			}
			break;
		case B:
			cout << "Estado B " << endl;
			if(this->received){// Recebeu frame do transceiver
				Framework::Type r = this->framework->receive();
				if(test_data(r)){
					this->currentstate = B;
				}else if(test_ack(r)){
					this->currentstate = C;
				}
			}else if(this->timeout){
				this->currentstate = D;
			}
			break;
		case C:
			cout << "Estado C " << endl;
			if(this->backoff){
				this->currentstate = A;
			}else if(this->received){
				Framework::Type r = this->framework->receive();
				test_data(r);
				this->currentstate = C;
			}
			break;
		case D:
			cout << "Estado D " << endl;
			if(this->backoff){
				if(!(this->framework->send(0,sequenceN)) > 0){
					cout << "Erro ao enviar quadro de sequencia: " << this->sequenceN << endl;
				}
				this->currentstate = B;
			}else if(this->received){
				Framework::Type r = this->framework->receive();
				if(test_data(r)){
					this->currentstate = D;
				}
			}
			break;
	}
	return false;
}

bool ARQ::test_data(Framework::Type r){

	if(r == Framework::data0){
		this->sequenceM = 0;
		if(!(this->framework->send(1,this->sequenceM) > 0)){
			cout << "Erro ao enviar quadro de sequencia: " << this->sequenceM;
		}
		return true;
	}else if(r == Framework::data1){
		this->sequenceM = 1;
		if(!(this->framework->send(1,this->sequenceM) > 0)){
			cout << "Erro ao enviar quadro de sequencia: " << this->sequenceM;
		}
		return true;
	}else{
		cout << "Erro: tipo de mensagem não coerente" << endl;
	}
	return false;
}

bool ARQ::test_ack(Framework::Type r){

	if(r == Framework::ack0){
		if(sequenceN == 0){
			sequenceN = 1;
			return true;
		}
	}else if(r == Framework::ack1){
		if(sequenceN == 1){
			sequenceN = 0;
			return true;
		}
	}

	return false;
}


