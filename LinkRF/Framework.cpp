/*
 * Framming.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Authors: Mathias Silva da Rosa
 *		 Vinicius Bandeira
 *		 Guilherme de Albuquerque
 */

#include "Framework.h"
#include <bitset>

Framework::Framework(Serial & tr, Serial & app, int bytes_min, int bytes_max): transceiver(tr), aplicacao(app) {

	this->min_bytes = bytes_min;
	this->max_bytes = bytes_max;
	this->buffer = new char[BUFSIZE];
	this->n_bytes = 0;
	this->currentState = waiting;
	CRC crc;
	this->crc = &crc;

}

int Framework::send(int type, int seq){

	cout << "INFO: Início do enquadramento..." << endl;

	char *buffer_payload = new char[BUFSIZE]; // Pode receber um dado de 1024 bytes da aplicação

	int n;

	if(type != 1){ // Se for payload recebe da aplicação
		off_t fsize = lseek(this->aplicacao.get_fd(),0,SEEK_END);
		lseek(this->aplicacao.get_fd(),0L,0);
		if((n = (this->aplicacao.read(buffer_payload,fsize,true))) > 0){
			cout << "INFO: Leitura do descritor da aplicação: " << this->aplicacao.get_fd() << " com sucesso!" << endl;
		}else{
			cout << "ERRO: Falha na leitura do descritor da aplicação: " << this->aplicacao.get_fd() << endl;
			return -1;
		}
	}else{ 									// Se não é ACK
		*buffer_payload = 'a'; //Byte de enchimento
		 n = this->min_bytes;
	}

	char* frame;

	if (n >= this->min_bytes && n <= this->max_bytes) {
		frame = mount(buffer_payload,n,type, seq);
		cout << "Quadro: " << frame << endl;
		int k;
		if((k = transceiver.write(frame, strlen(frame)) > 0)){
			lseek(this->transceiver.get_fd(),0L,0);
			cout << "INFO: Escrita no descritor do transceiver: " << this->transceiver.get_fd() << " com sucesso!" << endl;
		}else{
			cout << "ERRO: Falha na escrita no descritor do transceiver: " << this->transceiver.get_fd() << endl;
			return -1;
		}
	} else {
		cout << "ERRO: Quadro excedeu o limite máximo de envio: " << n << endl;
		return -1;
	}

	return 1;
}

char * Framework::mount(char * buffer, int len, int type, int seq){

	char* frame =  new char[FRAME_MAXSIZE];

	frame[0] = 0x7E; // Flag de início de quadro

	frame[1] = header(type, seq);

	int i,j;
	for( i=0, j=2 ; i < len ; i++,j++ ){
		switch(buffer[i]){
			case(0x7E):
				frame[j] = 0x7D;
				frame[j+1] = 0x5E;
				j = j + 1;
				cout << "INFO: Flag 0x7E na posição "<< i << "do dado!" << endl;
				break;
			case(0x7D):
				frame[j] = 0x7D;
				frame[j+1] = 0x5D;
				j = j + 1;
				cout << "INFO: Flag 0x7D na posição "<< i << "do dado!" << endl;
				break;
			default:
				frame[j] = buffer[i];
		}
	}

    this->crc->gen_crc((unsigned char *)frame+2,j-2);

//	frame[j+1] = 0x7E; // Flag de fim de quadro
//	frame[j+2] = 0; // Delimitador de char

	frame[j] = 0x7E; // Flag de fim de quadro
	frame[j+1] = 0; // Delimitador de char

	return frame;
}

Framework::Type Framework::receive(){

	cout << "INFO: Início da remoção do enquadramento..." << endl;

	char * frame = new char[FRAME_MAXSIZE];

	bool return_fsm;
	char frame_byte;
	int n = 0, k = 0, v = 0;

	bool end_flag  = false;
	while(!end_flag){
		if(k >= FRAME_MAXSIZE){
			cout << "ERRO: Quadro excedeu o limite máximo de recepção: " << k << endl;
			return Framework::none;
		}
		if(!(n = this->transceiver.read(frame+k,1, true)) > 0){
			cout << "ERRO: Erro ao ler byte do transceiver: " << n << std::endl;
			return Framework::none;
		}else{
			k += n;
			if(frame[k-1] == '~'){
				v++;
				if(v >= 2){
					end_flag = true;
				}
			}
		}
	}
	cout << "INFO: Leitura do descritor do transceiver: " << this->transceiver.get_fd() << " com sucesso!" << endl;

	frame[k] = 0; // Delimitador de char

//	unsigned char * crc_buffer = new unsigned char[FRAME_MAXSIZE-2];
//
//	int i = 0;
//	while( frame[i+1] != '~' ){
//		crc_buffer[i] = frame[i+1];
//		i++;
//	}
//
//	unsigned char * crc_buffer2 = new unsigned char[FRAME_MAXSIZE-2];
//	memcpy(crc_buffer2,crc_buffer,i);
//
//	crc_buffer2[i-2] = 0;
//	crc_buffer2[i-3] = 0xf0;
//	crc_buffer2[1-4] = 0xb8;
//	cout << crc_buffer2[i-3] << endl;
//	cout << crc_buffer2[i-4] << endl;
//
//
//	if(!(this->crc->check_crc(crc_buffer2,i-2))){
//		std::cout << "CRC does not match!: " << buffer  << std::endl;
//		return Framework::none;
//	}

	for(int i=0;!return_fsm; i++){
		frame_byte = frame[i];
		return_fsm = this->handle(frame_byte);
	}

	Framework::Type r = get_type(this->buffer[0]);

	if((n = this->aplicacao.write(this->buffer+1,strlen(this->buffer)-1))){
		lseek(this->aplicacao.get_fd(),0L,0);
		cout << "INFO: Escrita no descritor da aplicação: " << this->aplicacao.get_fd() << " com sucesso!" << endl;
	}else{
		cout << "ERRO: Falha na escrita no descritor do aplicação: " << this->aplicacao.get_fd() << endl;
		return Framework::none;
	}

	return r;
}

bool Framework::handle(char byte){

	switch(this->currentState){

	case waiting:
		if(byte == 0x7E){
			cout << "INFO: Flag de início de quadro encontrada!" << endl;
			this->n_bytes = 0;
			this->currentState = reception;
		} else {
			this->currentState = waiting;
		}
		break;
	case reception:
		if (this->n_bytes > this->max_bytes){
			this->currentState = waiting;
		} else {
			if(byte == 0x7E){
				std::cout << "INFO: Flag de fim de quadro encontrada!" << std::endl;
				this->currentState = waiting;
				this->buffer[n_bytes] = 0;
				return true; // frame finished
			} if (byte == 0x7D) {
				this->currentState = escape;
			} else {
				this->buffer[n_bytes] = byte;
				this->n_bytes++;
				this->currentState = reception;
			}
		}
		break;
	case escape:
		switch(byte){
		case(0x5E):
			std::cout << "INFO: 0x7E encontrado na posição: " << n_bytes << " do dado" << std::endl;
    		this->buffer[n_bytes] = 0x7E;
    		this->n_bytes++;
			this->currentState = reception;
			break;
		case(0x5D):
			std::cout << "INFO: 0x7D encontrado na posição: " << n_bytes << " do dado" << std::endl;
			this->buffer[n_bytes] = 0x7D;
			this->currentState = reception;
			break;
		case(0x7E):
				std::cout << "INFO: Flag de fim de quadro encontrada!" << std::endl;
				this->currentState = waiting;
				return true; // frame finished
		default:
			this->buffer[n_bytes] = byte;
			this->currentState = reception;
			break;
		}
		break;
	}
	return false;
}

char Framework::header(int type, int seq){

	char control;

	if(type == 0 && seq == 0){ // Tipo Dado, Sequência 0;
		control = '0';
	}

	if(type == 0 && seq == 1){ // Tipo Dado, Sequência 1;
		control = '1';
	}

	if(type == 1 && seq == 0){ // Tipo ACK, Sequência 0;
		control = '2';
	}

	if(type == 1 && seq == 1){ // Tipo ACK, Sequência 1;
		control = '3';
	}

	return control;

}

Framework::Type Framework::get_type(char control){

	switch(control){
		case('0'):
			return Framework::data0;
			break;
		case('1'):
			return Framework::data1;
			break;
		case('2'):
			return Framework::ack0;
			break;
		case('3'):
			return Framework::ack1;
			break;
		default:
			cout << "Erro: Tipo de mensagem não identificada!: " << control << endl;
			break;
	}
	return Framework::none;
}


