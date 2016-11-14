/*
 * Dispacher.cpp
 *
 *  Created on: 27 de out de 2016
 *      Author: mathias
 */

#include "Dispacher.h"


Dispacher::Dispacher(ARQ & a, int fd_transceiver, int fd_app):arq(a) {
	this->transceiver = fd_transceiver;
	this->aplicacao = fd_app;
	this->begin = 0;
}

Dispacher::~Dispacher() {}

void Dispacher::handle(){

	int op = fcntl(this->transceiver, F_GETFL);
	fcntl(this->transceiver, F_SETFL, op | O_NONBLOCK);

	struct timeval timeout;
	timeout.tv_sec = 20;
	timeout.tv_usec = 0;

	int maior = transceiver;
	if(aplicacao > transceiver) maior = aplicacao;

	while(true){
		// cria um conjunto de descritores
		fd_set r;

		// inicia o conjunto de descritores, e nele
		// acrescenta fd1 e fd2
		FD_ZERO(&r);
		FD_SET(this->transceiver, &r);
		FD_SET(this->aplicacao, &r);

		int n = 0;

		if( !(n = select(maior+1, &r, NULL, NULL, &timeout)) == 0 ){

			cout << "INFO:Há " << n << " descritores prontos" << endl;

			if(n < 0){
				perror("select()");
			}else if(n){

				// testa se fd1 está pronto para ser acessado
				if (FD_ISSET(this->transceiver, &r)) {
					this->arq.set_received(true);
					this->arq.handle();
					this->arq.set_received(false);
					this->begin = clock();
				}else{

					// Tempo de backoff

					//---------------------------------------------------------------

					clock_t end = clock();
					double elapsed_secs = double(end - this->begin) / CLOCKS_PER_SEC;
					if(elapsed_secs > this->arq.get_backoff()){
						this->arq.set_backoff(true);
						this->arq.handle();
					}

					//---------------------------------------------------------------
				}

				// testa se fd1 está pronto para ser acessado
				if (FD_ISSET(this->aplicacao, &r)) {
					this->arq.set_canSend(true);
					this->arq.handle();
					this->arq.set_canSend(false);
				}
			}
		}else{
			cout << "INFO: Nenhuma operação depois de 5 segundos!" << endl;
			this->arq.set_timeout(true);
			this->arq.handle();
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		}
	}
}
