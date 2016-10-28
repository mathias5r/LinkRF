/*
 * Dispacher.cpp
 *
 *  Created on: 27 de out de 2016
 *      Author: mathias
 */

#include "Dispacher.h"

Dispacher::Dispacher(ARQ & arq):arq(arq) {};

Dispacher::~Dispacher() {}

void Dispacher::handle(){

	int serial, app;

	serial = open("/dev/ttyUSB0", O_RDONLY, O_NONBLOCK);
	app = 0;

	int op = fcntl(app, F_GETFL);
	fcntl(app, F_SETFL, op | O_NONBLOCK);

	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	while(true){
		// cria um conjunto de descritores
		fd_set r;

		// inicia o conjunto de descritores, e nele
		// acrescenta fd1 e fd2
		FD_ZERO(&r);
		FD_SET(serial, &r);
		FD_SET(app, &r);

		int n;

		if( (n = select(serial+1, &r, NULL, NULL, &timeout)) == 0 ){
			this->arq.set_timeout(true);
			this->arq.handle((char*)"",0);
		}else{


			cout << "Ha " << n << " descritores prontos" << endl;

			// testa se fd1 está pronto para ser acessado
			if (FD_ISSET(serial, &r)) {
				char buffer[1024];
				cout << "Descritor do random: ";
				cout << read(serial, buffer, 1024) << " bytes" << endl;
				this->arq.set_received(true);
				this->arq.handle(buffer,1024);
			}

			// testa se fd1 está pronto para ser acessado
			if (FD_ISSET(app, &r)) {
				char buffer[1024];
				cout << "Descritor do teclado: ";
				cout << read(app, buffer, 1024) << " bytes" << endl;
				this->arq.set_canSend(true);
				this->arq.handle(buffer,1024);
			}
		}

	}
}
