/*
 * Dispacher.cpp
 *
 *  Created on: 27 de out de 2016
 *  
 *  Classe para monitoramento de recebimento de pacotes na interface TUN
 *  ou recebimento de quadros na interface serial (Transceiver).
 */

#include "Dispacher.h"

/*
 * Construtor da classe Dispatcher.
 *
 * Inicialização dos descritores das interfaces TUN e serial.
 *
 */
Dispacher::Dispacher(ARQ & a, int fd_transceiver, int fd_app):arq(a) {
	this->transceiver = fd_transceiver;
	this->aplicacao = fd_app;
	this->backoff_control = 0;
}

Dispacher::~Dispacher() {}

/*
 * Para o controle do envio e o recebimento de pacotes/quadros de ambas interface,
 * foi desenvolvido o método abaixo que segue como um árbitro controlando os pacotes/quadros 
 * que são recebidos e transmitidos.
 * O controle é feito através da função select, em que é verificado se há ou não descritores prontos. 
 * 
 * Há dois casos possíveis através do select: 
 * 
 *    ** Descritor da serial pronto: 
 *	       => Envia informação ao ARQ que foi recebido quadro da serial;
 *	       => Envia o quadro ao ARQ para o devido tratamento da camada acima.
 *
 *    ** Descritor da TUN pronto:
 *             => Envia informação ao ARQ que pode transmitir o pacote;
 *             => Envia o pacote ao ARQ para o devido tratamento para a transmissão.
 *
 * No caso em que há descritores prontos, ou seja, não há como receber ou transmitir dados
 * ainda é realizada a transmissão que é descrita logo abaixo:
 *
 *    ** Timeout 
 *            => Envia informação ao ARQ para "setar" o timeout como true;
 *            => É gerado um timeout aleatório para o backoff; 
 *            => O estado da FSM do ARQ correspondente tratará essa exceção.
 *
 *    ** Backoff
 *            => É realizada a retransmissão do quadro.
 *
 */
void Dispacher::handle(){

	int op = fcntl(this->transceiver, F_GETFL);
	fcntl(this->transceiver, F_SETFL, op | O_NONBLOCK);

	struct timeval timeout;
	timeout.tv_sec = 1;
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

			cout << "INFO: Há " << n << " descritores prontos" << endl;

			if(n < 0){
				perror("select()");
			}else if(n){

				// testa se fd1 está pronto para ser acessado
				if (FD_ISSET(this->transceiver, &r)) {
					this->arq.set_received(true);
					this->arq.handle();
					this->arq.set_received(false);
				}

				// testa se fd1 está pronto para ser acessado
				if (FD_ISSET(this->aplicacao, &r)) {
					this->arq.set_canSend(true);
					this->arq.handle();
					this->arq.set_canSend(false);
				}
			}
		}else{

			cout << "INFO: Nenhuma operação depois de 1 segundo!" << endl;

			this->arq.set_timeout(true);
			this->arq.handle();
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			if(this->arq.get_enable()){
				this->backoff_control++;
			}

			cout << "INFO: Retransmissão em: " << this->arq.get_backoff() << " segundos - " << this->backoff_control << endl;

			if(this->backoff_control >= this->arq.get_backoff()){
				this->backoff_control = 0;
				this->arq.set_backoff(true);
				this->arq.handle();
				this->arq.set_backoff(false);
			}
		}
	}
}
