/*
 * main.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Author: aluno
 */
#include "Serial.h"
#include "File.h"
#include <iostream>

#include "Framework.h"

void receive_msg(char* buffer){

	Serial s("/dev/ttyUSB0",B9600);
	Framework f(s,8,BUFSIZE);

	f.receive(buffer);
}

void send_msg(char *buffer){

	Serial s("/dev/ttyUSB0",B9600);
	Framework f(s,8,BUFSIZE);

	f.send(buffer, strlen(buffer));
}

int main(){

	std::cout << "File Transfer Aplication\n";

	File f((char*)"/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/texto.txt");

	int n = 0;
	char* buffer_text = new char[f.getSize()];

	f.readfile(buffer_text,f.getSize(),n);

	std::cout << "Buffer Text: " << buffer_text << std::endl;

	std::cout << "SIZE: " << strlen(buffer_text) << std::endl;

	char* msg, *buffer = new char[BUFSIZE];
	msg = (char*)"this is a }^}]}^}}]]^^~~ test message";

	receive_msg(buffer);
	std::cout << "Received data: " << buffer << std::endl;

	//send_msg(msg);
	//std::cout << "Data sent" << std::endl;

}






