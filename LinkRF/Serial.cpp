/*
 * Serial.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Author: aluno
 */
#include "Serial.h"

Serial::Serial(const char * path, int rate) {

	/* Open modem device for reading and writing and not as controlling tty */
	this->tty_fd = open(path,O_RDWR|O_NOCTTY);
	if (tty_fd < 0) throw -10;
	config(this->tty_fd, rate);

}


Serial::Serial(int fd, int rate){

	this->tty_fd = fd;
	config(this->tty_fd, rate);

}

Serial::~Serial() {
}

/* 
 * Escrita dos quadros na interface serial,
 * tendo origem a interface TUN. Ou seja,
 * tranmissão dos quadros.
 *
 */
int Serial::write(char * buffer, int len){

	int pos = 0;
	while (pos < len) {
		int bytes = len - pos;
		if (bytes > MAX_SERIAL_BYTES) bytes = MAX_SERIAL_BYTES;
		int sent = ::write(tty_fd, buffer+pos, bytes);
		pos += sent;
		::tcdrain(tty_fd);
	}
	return pos;
}

/*
 * Método de leitura dos quadros
 * na interface serial.
 *
 */
int Serial::read(char * buffer, int len){

	return read(buffer, len, false);
}

/*
 * Método de leitura bloqueante dos 
 * quadros na interface serial.
 *
 */
int Serial::read(char * buffer, int len, bool block){
	if (block) {
		fd_set r;

		FD_ZERO(&r);
		FD_SET(tty_fd, &r);
		select (tty_fd+1, &r, NULL, NULL, NULL);
	}
	int n = ::read(tty_fd, buffer, len);
	return n;
}

char Serial::read_byte() {
	char c;
	read(&c, 1, false);
	return c;
}

/*
 * Retorna o valor do descritor da serial.
 *
 */
int Serial::get_fd(){
	return this->tty_fd;
}

/*
 * Configuração padrão da serial. Esse método será chamado
 * no construtor para inicializar a interface serial.
 *
 */
void Serial::config(int fd, int rate){

	struct termios tio;

	/* Save current serial port settings */
	tcgetattr(fd, &tio);

	/* initialize all control characters
		default values can be found in /usr/include/termios.h, and are given
		in the comments, but we don't need them here */
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = CS8|CREAD|CLOCAL;   // 8n1, see termios.h for more information
	tio.c_lflag = 0;
	tio.c_cc[0] = 3;
	tio.c_cc[1] = 0x1c;
	tio.c_cc[2] = 0x7f;
	tio.c_cc[3] = 0x15;
	tio.c_cc[4] = 4;
	tio.c_cc[5] = 0;
	tio.c_cc[6] = 0;
	tio.c_cc[7] = 0;
	tio.c_cc[8] = 0x11;
	tio.c_cc[9] = 0x13;
	tio.c_cc[10] = 0x1a;
	tio.c_cc[11] = 0;
	tio.c_cc[12] = 0x12;
	tio.c_cc[13] = 0xf;
	tio.c_cc[14] = 0x17;
	tio.c_cc[15] = 016;
	for (int i=16; i < 32; i++) tio.c_cc[i] = 0;

	/* Set the baurate of the serial communication */
	cfsetospeed(&tio,rate);
	cfsetispeed(&tio,rate);

	/* Restore the old port settings */
	tcsetattr(fd,TCSANOW,&tio);

	long flag;
	ioctl(fd, F_GETFL, &flag);
	flag |= O_NONBLOCK;
	ioctl(fd, F_SETFL, &flag);


}



