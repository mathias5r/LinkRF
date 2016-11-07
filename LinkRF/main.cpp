/*
 * main.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Author: aluno
 */
#include <iostream>

#include "Dispacher.h"

using namespace std;

int main(){

	Serial t("/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/serial.txt",B9600);
	Serial a("/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/aplicacao.txt",B9600); // Tun
	Framework f(t,a,8,2048);
	ARQ arq(f);
	Dispacher d(arq,t.get_fd(),a.get_fd());

	d.handle();

}






