/*
 * main.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Author: aluno
 */
#include "Serial.h"
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

	char* msg, *buffer = new char[BUFSIZE];
	msg = (char*)"this is a }^}]}^}}]]^^~~ test message";

	receive_msg(buffer);
	std::cout << "Received data: " << buffer << std::endl;

	/*send_msg(msg);
	std::cout << "Data sent" << std::endl;*/

}






