/*
 * ARQ.cpp
 *
 *  Created on: 20 de out de 2016
 *      Author: mathias
 */

#include "ARQ.h"

ARQ::ARQ(Framework & f): framework(f){

	this->currentstate = A;
	this->sequenceN = 0;
	this->sequenceM = 0;
	this->canSend = false;
	this->received = false;
	this->backoff = false;
	this->timeout = false;

}

bool ARQ::handle(){

	switch(this->currentstate){

	case A:

		cout << "INFO: Estado ARQ: A " << endl;

		if(this->canSend){ 								// Recebeu um dado da aplicação
			if((this->framework.send(0,this->sequenceN)) > 0){
				cout << "INFO: Dado com sequência " << this->sequenceN << " Enviado com sucesso!" << endl;
				this->currentstate = B;
			}else{
				cout << "ERRO: Erro ao enviar dado com sequência " << this->sequenceN << endl;
				return true;
			}
		}else if(this->received){						// Recebeu um quadro do transceiver
			Framework::Type r;
			if((r = this->framework.receive()) != Framework::none){
				if(test_data(r)){
					this->currentstate = A;
				}else{
					cout << "ERRO: Tipo de dado não coerente!" << endl;
					return true;
				}
			}else{
				return true;
			}
		}else{
			cout << "ERRO: Operação do estado A inválido" << endl;
			return true;
		}

		break;

	case B:

		cout << "INFO: Estado ARQ: B " << endl;

		if(this->received){								// Recebeu um quadro do transceiver
			Framework::Type r;
			if((r = this->framework.receive()) != Framework::none){
				if(test_data(r)){
					this->currentstate = B;
				}else if(test_ack(r)){
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado ou confirmação não coerente!" << endl;
					return true;
				}
			}
		}else if(this->timeout){
			this->currentstate = D;
		}else{
			cout << "ERRO: Operação do estado B inválido" << endl;
			return true;
		}

		break;

	case C:

		cout << "INFO: Estado ARQ: C " << endl;

		if(this->backoff){
			this->currentstate = A;
		}else if(this->received){
			Framework::Type r;
			if((r = this->framework.receive()) != Framework::none){
				if(test_data(r)){
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado não coerente!" << endl;
					return true;
				}
			}
		}else{
			cout << "ERRO: Operação do estado C inválido" << endl;
			return true;
		}

		break;

	case D:

		cout << "INFO: Estado ARQ: D " << endl;

		if(this->backoff){
			if((this->framework.send(0,sequenceN)) > 0){
				cout << "INFO: Dado com sequência " << this->sequenceN << "Enviado com sucesso!" << endl;
				this->currentstate = B;
			}else{
				cout << "ERRO: Erro ao enviar quadro de sequencia: " << this->sequenceN << endl;
				return true;
			}
		}else if(this->received){
			Framework::Type r = this->framework.receive();
			if(test_data(r)){
				this->currentstate = D;
			}else{
				cout << "ERRO: Tipo de dado não coerente!" << endl;
				return true;
			}
		}

		break;

	}
	return false; // Não ocorreu problema
}


bool ARQ::test_data(Framework::Type r){

	if(r == Framework::data0){
		this->sequenceM = 0;
		if((this->framework.send(1,this->sequenceM) > 0)){
			cout << "INFO: ACK com sequência " << this->sequenceM << " Enviado com sucesso!" << endl;
			return true;
		}else{
			cout << "Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
			return false;
		}
	}else if(r == Framework::data1){
		this->sequenceM = 1;
		if((this->framework.send(1,this->sequenceM) > 0)){
			cout << "INFO: ACK com sequência " << this->sequenceM << "Enviado com sucesso!" << endl;
			return true;
		}else{
			cout << "Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
			return false;
		}
	}
	return false;
}

bool ARQ::test_ack(Framework::Type r){

	if(r == Framework::ack0){
		if(sequenceN == 0){
			sequenceN = 1;
			cout << "INFO: ACK recebido de sequência: " << this->sequenceN << endl;
			return true;
		}else{
			cout << "ERRO: Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
		}
	}else if(r == Framework::ack1){
		if(sequenceN == 1){
			sequenceN = 0;
			return true;
		}
	}
	return false;
}


