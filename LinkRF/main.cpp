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

	cout << "------------------------- Aplicação LinkRF ------------------------------- " << endl;
	//cout << "--------Configuração---------------- " << endl
	//cout << " Digite a interface serial:

	Tun iface("ptp0" , "10.0.0.1", "10.0.0.2");
	iface.start();
	Serial t("/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/serial.txt",B9600);
	Serial a(iface.get(),B9600); // Tun
	Framework f(t,a,1,FRAME_MAXSIZE);
	ARQ arq(f);
	Dispacher d(arq,t.get_fd(),a.get_fd());

	d.handle();

}






