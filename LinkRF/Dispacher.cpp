/*
 * Dispacher.cpp
 *
 *  Created on: 27 de out de 2016
 *      Author: mathias
 */

#include "Dispacher.h"

Dispacher::Dispacher() {

	Serial t("/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/serial.txt",B9600);
	Serial a("/home/mathias/Workspace_Eclipse/LinkRF/LinkRF/aplicacao.txt",B9600); // Tun

	this->trans = &t;
	this->app = &a;
	this->transceiver = this->trans->get_fd();
	this->aplicacao  = this->app->get_fd(); // tun.get_fd();

	Framework f(this->trans,this->app,8,2048);

	this->framework = &f;

	ARQ arq(framework,this->transceiver, this->aplicacao);

	this->arq = &arq;
}

Dispacher::~Dispacher() {}

void Dispacher::handle(){

//	int op = fcntl(this->aplicacao, F_GETFL);
//	fcntl(this->aplicacao, F_SETFL, op | O_NONBLOCK);

//	struct timeval timeout;
//	timeout.tv_sec = 5;
//	timeout.tv_usec = 0;

	while(true){
		// cria um conjunto de descritores
		fd_set r;

		// inicia o conjunto de descritores, e nele
		// acrescenta fd1 e fd2
		FD_ZERO(&r);
		FD_SET(this->transceiver, &r);
		FD_SET(this->aplicacao, &r);

		int n;

		cout << "Ha " << n << " descritores prontos" << endl;

		if( (n = select(transceiver+1, &r, NULL, NULL, NULL)) == 0 ){

			// testa se fd1 está pronto para ser acessado
			if (FD_ISSET(this->transceiver, &r)) {
				this->arq->set_received(true);
				this->arq->handle();
			}

			// testa se fd1 está pronto para ser acessado
			if (FD_ISSET(this->aplicacao, &r)) {
				this->arq->set_canSend(true);
				this->arq->handle();
			}
		}
	}
}
