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

	char  interface[256];
	char  ip_local[256];
	char  ip_destiny[256];
	char  transceiver[256];

	cout << "------------------------- Protocolo de Comunicação ------------------------------- " << endl;
	cout << "-- Configuração da Rede:---------------------------------------------------------- " << endl;
	cout << "-- Digite o nome da interface: " << endl;
	cin.getline(interface,256);
	cout << "-- Digite o IP local: " << endl;
	cin.getline(ip_local,256);
	cout << "-- Digite o IP de destino: " << endl;
	cin.getline(ip_destiny,256);
	cout << "-- Digite o caminho do transceiver: " << endl;
	cin.getline(transceiver,256);

	cout << "-------------------------------- Configuração ------------------------------------ " << endl;
	cout << " Interface: " << interface << endl;
	cout << " IP local: " << ip_local << endl;
	cout << " Ip de destino: " << ip_destiny << endl;
	cout << " Transceiver: " << transceiver << endl;

	cout << "---------------------------  Iniciando Comunicação ------------------------------- " << endl;
	cout << "-- Realizar ping no Ip de destino! " << endl;
	cout << "" << endl;
	cout << "" << endl;

	Tun iface(interface,ip_local,ip_destiny);
	iface.start();
	Serial t(transceiver,B9600);
	Framework f(t,1,FRAME_MAXSIZE);
	ARQ arq(f, iface);
	Dispacher d(arq,t.get_fd(),iface.get());

	d.handle();

}






