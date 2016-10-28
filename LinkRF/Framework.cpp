/*
 * Framming.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Authors: Mathias Silva da Rosa
 *		 Vinicius Bandeira
 *		 Guilherme de Albuquerque
 */

#include "Framework.h"

Framework::Framework(Serial & s, int bytes_min, int bytes_max):serial(s) {

	this->min_bytes = bytes_min;
	this->max_bytes = bytes_max;
	this->buffer = new char[BUFSIZE];
	this->n_bytes = 0;
	this->currentState = waiting;
	CRC crc;
	this->crc = &crc;

}

int Framework::send(char *buffer, int len, int type, int seq){

	cout << "Início do enquadramento..." << endl;

	char* frame;

	if (len >= this->min_bytes && len <= this->max_bytes) {
		frame = mount(buffer,len, type, seq);
		cout << "Quadro: " << frame << endl;
		int n;
		if(!(n = serial.write(frame, strlen(frame)) > 0)){
			cout << "Erro ao escrever quadro na serial: " << n << endl;
			return -1;
		}
	} else {
		cout << "Erro: quadro excedeu o limite máximo!" << endl;
		return -1;
	}

	return 1;
}

char * Framework::mount(char* data, int len, int type, int seq){

	//2*BUFSIZE+5 = Flags + False Flags + Data + CRC
	char* frame =  new char[FRAME_MAXSIZE];

	frame[0] = 0x7E; // Flag de início de quadro

	frame[1] = header(type, seq);

	int i,j;
	for( i=1, j=2 ; i <= len ; i++,j++ ){
		switch(data[i]){
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
				frame[j] = data[i];
		}
	}

	this->crc->gen_crc((unsigned char *)frame+1,j-2);

	frame[j+1] = 0x7E; // Flag de fim de quadro
	frame[j+2] = 0; // Delimitador de char

	return frame;
}

int Framework::receive(char* buffer){

	cout << "Início da remoção do enquadramento..." << endl;

	bool return_fsm;
	char frame[FRAME_MAXSIZE];
	char frame_byte;
	int n = 0, k = 0, v = 0;

	bool end_flag  = false;
	while(!end_flag){
		if(k >= FRAME_MAXSIZE){
			cout << "Quadro excedeu o limite máximo de bytes!: " << k << endl;
			return -1;
		}
		if(!(n = this->serial.read(frame+k,1, true)) > 0){
			std::cout << "Erro ao ler byte do quadro:  " << n << std::endl;
			return -1;
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

	frame[k] = 0;

	if(!(this->crc->check_crc((unsigned char*)frame+1,k-3))){
		std::cout << "CRC does not match!: " << buffer  << std::endl;
		return -1;
	}

	for(int i=0;!return_fsm; i++){
		frame_byte = frame[i];
		return_fsm = this->handle(frame_byte);
	}

	memcpy(buffer, this->buffer, BUFSIZE);

	return this->n_bytes;
}

bool Framework::handle(char byte){

	switch(this->currentState){

	case waiting:
		if(byte == 0x7E){
			std::cout << "Initial delimitation found!" << std::endl;
			this->n_bytes = 0;
			this->currentState = reception;
		} else {
			this->currentState = waiting;
		}
		break;

	case reception:
		if (this->n_bytes > this->max_bytes){
			std::cout << "Overflow: " << this->n_bytes << " bytes" << std::endl;
			this->currentState = waiting;
		} else {
			if(byte == 0x7E){
				std::cout << "End delimitation found!" << std::endl;
				this->currentState = waiting;
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
			std::cout << "INFO: 0x7E found in position: " << n_bytes << " of the data" << std::endl;
    		this->buffer[n_bytes] = 0x7E;
			this->currentState = reception;
			break;
		case(0x5D):
			std::cout << "INFO: 0x7D found in position: " << n_bytes << " of the data" << std::endl;
			this->buffer[n_bytes] = 0x7D;
			this->currentState = reception;
			break;
		case(0x7E):
				std::cout << "End delimitation found!" << std::endl;
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

	char control = 0x00;

	if(type == 0 && seq == 0){ // Tipo Dado, Sequência 0;
		control &= ~(1 << 2);
		control &= ~(1 << 1);
	}

	if(type == 1 && seq == 0){ // Tipo ACK, Sequência 0;
		control |= (1 << 2);
		control &= ~(1 << 1);
	}

	if(type == 0 && seq == 1){ // Tipo Dado, Sequência 1;
		control &= ~(1 << 2);
		control |= (1 << 1);
	}

	if(type == 1 && seq == 1){ // Tipo ACK, Sequência 1;
		control |= (1 << 2);
		control |= (1 << 1);
	}

	return control;

}


