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

using namespace std;

void receive_msg(char* buffer){

	Serial s("/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/texto.txt",B9600);
	Framework f(s,FRAME_MINSIZE,FRAME_MAXSIZE);

	int n;
	if(!( n = f.receive(buffer)) > 0){
		cout << "Erro ao receber dado da serial: " << n << endl;
	}else{
		cout << "Dado recebido: " << buffer << endl;
	}
}

void send_msg(char *buffer){

	Serial s("/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/texto.txt",B9600);
	Framework f(s,FRAME_MINSIZE,FRAME_MAXSIZE);

	f.send(buffer, strlen(buffer),0,0);
}

int main(){

	char* msg, *buffer = new char[BUFSIZE];
	msg = (char*)"this is a }^}]}^}}]]^^~~ test message";

	receive_msg(buffer);

	//send_msg(msg);
	//std::cout << "Data sent" << std::endl;

}






