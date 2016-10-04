/*
 * Serial.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Author: aluno
 */
#include "Serial.h"

Serial::Serial(const char * path, int rate) {

	struct termios tio;

	this->tty_fd = open(path,O_RDWR|O_NOCTTY);

	tcgetattr(tty_fd, &tio);
	tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information

	cfsetospeed(&tio,rate);            // 9600 baud
	cfsetispeed(&tio,rate);            // 9600 baud

	tcsetattr(tty_fd,TCSANOW,&tio);
}

Serial::~Serial() {
}

int Serial::write(char * buffer, int size){

	int n = ::write(tty_fd, buffer, size);

	tcdrain(tty_fd);
	sleep(1);

	return n;
}

int Serial::read(char * buffer, int size){

	int n = ::read(tty_fd, buffer,BUFSIZE-1);

	if(n > 0 ){
		buffer[n] = 0;
	}else{
		perror("failed to read the buffer...");
	}

	return n;
}

int Serial::read(char * buffer, int len, bool block){

	int n = 0;

	char *buffer_aux = new char[len];

	if(block){
		std::cout<< "Waiting for data with block" << std::endl;
		while(n <= 10){
			n += ::read(tty_fd, buffer_aux,len);
			std::cout << "Buffer_aux: " << buffer_aux << std::endl;
		}
	}else{
		std::cout << "Waiting for data without block" << std::endl;
		n = ::read(tty_fd, buffer,len);
	}

	memcpy(buffer,buffer_aux,len);

//	if(n > 0 ){
//		buffer[n+1] = 0 ;
//	}else{
//		perror("failed to read the buffer...");
//	}

	delete[] buffer_aux;

	return n;
}




