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

	Tun iface("ptp0" , "192.168.1.1", "192.168.1.2");
	iface.start();
	Serial t("/dev/ttyUSB0",B9600);
	//Serial t("/dev/ttyUSB0",B9600);
	Framework f(t,1,FRAME_MAXSIZE);
	ARQ arq(f, iface);
	Dispacher d(arq,t.get_fd(),iface.get());

	d.handle();

}






