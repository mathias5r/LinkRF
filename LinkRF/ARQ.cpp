/*
 * ARQ.cpp
 *
 *  Created on: 20 de out de 2016
 *      Author: mathias
 */

#include "ARQ.h"

ARQ::ARQ(Framework & f, Tun & tun): framework(f), tun(tun) {

	this->currentstate = A;
	this->sequenceN = 0;
	this->sequenceM = 0;
	this->canSend = false;
	this->received = false;
	this->backoff = false;
	this->timeout = false;
	srand((unsigned)time(0)); //para gerar números aleatórios reais.
	int maior = 30;
	int menor = 20;
	int aleatorio = rand()%(maior-menor+1) + menor;
	this->backoff_value = aleatorio;
	this->enable = false;
}

bool ARQ::handle(){

	switch(this->currentstate){

	case A:

		cout << "INFO: Estado ARQ: A " << endl;

		if(this->canSend){ 								// Recebeu um dado da aplicação
			char * from_app = new char[BUFSIZE];
			Frame * frame;
			frame = this->tun.get_frame();
			frame->copy(from_app);
			int frame_size = frame->total_length();
			this->current_frame = *frame;
			if((((this->framework.send(from_app,frame_size,0,this->sequenceN))))){
				cout << "INFO: Dado com sequência " << this->sequenceN << " Enviado com sucesso!" << endl;
				this->currentstate = B;
			}else{
				cout << "ERRO: Erro ao enviar dado com sequência " << this->sequenceN << endl;
				return true;
			}
			delete from_app;
		}else if(this->received){						// Recebeu um quadro do transceiver
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = A;
				}else{
					cout << "ERRO: Tipo de dado não coerente: A!" << endl;
					return true;
				}
			}else{
				return true;
			}
			delete from_serial;
		}else{
			cout << "AVISO: Operação do estado A inválido" << endl;
			return true;
		}

		break;

	case B:

		cout << "INFO: Estado ARQ: B " << endl;
		this->enable = false;

		if(this->received){								// Recebeu um quadro do transceiver
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = B;
				}else if(test_ack(r)){
					srand((unsigned)time(0)); //para gerar números aleatórios reais.
					int maior = 30;
					int menor = 20;
					int aleatorio = rand()%(maior-menor+1) + menor;
					this->enable = true;
					this->backoff_value = aleatorio;
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado ou confirmação não coerente: B!" << endl;
					return true;
				}
			}
			delete from_serial;
		}else if(this->timeout){
			srand((unsigned)time(0)); //para gerar números aleatórios reais.
			int maior = 30;
			int menor = 20;
			int aleatorio = rand()%(maior-menor+1) + menor;
			this->enable = true;
			this->backoff_value = aleatorio;
			this->currentstate = D;
		}else{
			cout << "AVISO: Operação do estado B inválido" << endl;
			return true;
		}

		break;

	case C:

		cout << "INFO: Estado ARQ: C " << endl;

		if(this->backoff){
			this->enable = false;
			this->currentstate = A;
		}else if(this->received){
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado não coerente: C!" << endl;
					return true;
				}
			}
			delete from_serial;
		}else{
			cout << "AVISO: Operação do estado C inválido" << endl;
			return true;
		}

		break;

	case D:

		cout << "INFO: Estado ARQ: D " << endl;

		if(this->backoff){

			cout << "INFO: Retransmissão" << endl;

			//--------------------------------------------------------------------

			char * from_app = new char[BUFSIZE];
			this->current_frame.copy(from_app);
			int frame_size = this->current_frame.total_length();
			if((((this->framework.send(from_app,frame_size,0,this->sequenceN))))){
				cout << "INFO: Dado com sequência " << this->sequenceN << " Enviado com sucesso!" << endl;
				this->currentstate = B;
			}else{
				cout << "ERRO: Erro ao enviar dado com sequência " << this->sequenceN << endl;
				return true;
			}

			//--------------------------------------------------------------------

		}else if(this->received){
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = D;
				}else if(test_ack(r)){
					srand((unsigned)time(0)); //para gerar números aleatórios reais.
					int maior = 30;
					int menor = 20;
					int aleatorio = rand()%(maior-menor+1) + menor;
					this->enable = true;
					this->backoff_value = aleatorio;
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado não coerente: D!" << endl;
					return true;
				}
			}
			delete from_serial;
		}else{
			cout << "AVISO: Operação do estado D inválido" << endl;
			return true;
		}

		break;

	}
	return false; // Não ocorreu problema
}


bool ARQ::test_data(Framework::Type r){

	if(r == Framework::data0){
		this->sequenceM = 0;
		if((this->framework.send((char *)"a",1,1,this->sequenceM) > 0)){
			cout << "INFO: ACK com sequência " << this->sequenceM << " Enviado com sucesso!" << endl;
			return true;
		}else{
			cout << "ERRO: Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
			return false;
		}
	}else if(r == Framework::data1){
		this->sequenceM = 1;
		if((this->framework.send((char *)"a",1,1,this->sequenceM) > 0)){
			cout << "INFO: ACK com sequência " << this->sequenceM << "Enviado com sucesso!" << endl;
			return true;
		}else{
			cout << "ERRO: Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
			return false;
		}
	}
	return false;
}

bool ARQ::test_ack(Framework::Type r){

	if(r == Framework::ack0){
		if(sequenceN == 0){
			cout << "INFO: ACK recebido de sequência: " << this->sequenceN << endl;
			sequenceN = 1;
			return true;
		}else{
			cout << "ERRO: Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
		}
	}else if(r == Framework::ack1){
		if(sequenceN == 1){
			cout << "INFO: ACK recebido de sequência: " << this->sequenceN << endl;
			sequenceN = 0;
			return true;
		}
	}
	return false;
}


