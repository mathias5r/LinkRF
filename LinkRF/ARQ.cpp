/*
 * ARQ.cpp
 *
 *  Created on: 20 de out de 2016
 *   
 *  Classe do mecanismo ARQ pare-e-espere 
 */

#include "ARQ.h"


/*
 * Construtor da classe ARQ.
 *
 * Por default é gerado o time do backoff
 *  
 */
ARQ::ARQ(Framework & f, Tun & tun): framework(f), tun(tun) {

	this->currentstate = A;
	this->sequenceN = 0;
	this->sequenceM = 0;
	this->canSend = false;
	this->received = false;
	this->backoff = false;
	this->timeout = false;
	srand((unsigned)time(0)); //para gerar números aleatórios reais.
	int maior = 5;
	int menor = 1;
	int aleatorio = rand()%(maior-menor+1) + menor;
	this->backoff_value = aleatorio;
	this->enable = false;
}

/*
 * Implementação da FSM (Finity State Machine) para o controle do ARQ.
 *
 * Caso A: Recebeu um pacote da aplicação | Recebeu um quadro da serial.
 *	
 *      ** Verifica se recebeu um dado da app ou da interface serial
 *         através da sinalização enviada pelo Dispatcher.
 *		*** Transmissão:
 *       		=> Recebe o dado de origem da interface TUN e o 
 *			   transmite através da interface serial;
 *			=> Segue ao próximo estado (B).
 *              *** Recepção:
 *			=> Recebe o dado de origem da interface serial e
 *			   o envia para as camadas superiores, ou seja,
 *			   para a TUN, sendo assim enviado a app;
 *			=> Mantém no mesmo estado (A) para a transmissão		
 *			   ou recepção de dados. 
 *  
 *      ** Caso não ocorra nenhum das situações citadas acima, apenas é  
 *	   gerado um erro default com "Operação inválida".
 *
 * Caso B:
 *
 *      ** Verifica se recebeu algum dado. Caso não tenha recebido, é gerado 
 *         um timeout para o backoff (retransmissão).
 *		*** Recepção:
 *			=> Verificação se é data0, data1, ack0, ack1. Caso não
 *			   seja o dado é deletado.  
 *			   **** Verifica se é um dado(0|1) ou ack(0|1):
 *				=> dado: Transmite o dado de origem da app e
 *					 mantém no estado atual (B) na espera
 *					 do ack.
 *				=> ack: Recebe o ack e segue para o próximo
 *					estado (C). Aonde é setado um timeout
 *					de backoff e volta ao estado inicial (A).
 *		*** Timeout:
 *			=> Gera um timeout aleatório, habilita o backoff e segue 
 *			   para o próximo estado (D) para retransmissão.
 *
 *
 * Caso C:
 *  	
 * 	** Verifica se o backoff está habilitado e verifica também se houve recepção 
 *	   de dados. 
 *   		*** Backoff:
 *			=> Desabilita o backoff;
 *			=> Retorna ao estado inicial (A) para transmissão e recepção
 *			   de pacotes/quadros.
 *
 * 		*** Recepção:
 *			=> Recebe quadro de origem da serial;
 *			=> Envia a camada superior, ou seja, a interface TUN que será enviado para app;
 *			=> Mantém no mesmo estado (C) para recepção de mais quadros.
 *
 *
 * Caso D:
 *
 *	** Verifica se o backoff foi habilitado devido algum erro de transmissão. É 
 *         verififcado também se houve algum sinal de origem do Dispatcher avisando
 *	   que há quadro(s) na recepção.
 *		*** Backoff:
 *			=> Transmite para interface serial o pacote de origem
 *			   da TUN (origem inicial app);
 *			=> Retorna ao estado (B) para ficar à espera de data(0|1)
 *                         ou ack(0|1).
 *
 *
 *		*** Recepção:
 *			=> Fica à espera na recepção de algum dado;
 *			=> Verifica se o dado recebido é um data(0|1) 
 *			   ou ack(0|1);
 *			=> Caso seja data(0|1) envia o dado à camada
 *			   superior. Caso seja um ack(0|1) gera um valor
 *			   valor aleatório para o time do backoff e retorna 
 *			   ao estado (C).
 */
bool ARQ::handle(){

	switch(this->currentstate){

	case A:

		cout << "INFO: Estado ARQ: A " << endl;

		if(this->canSend){ 								// Recebeu um dado da aplicação
			char * from_app = new char[BUFSIZE];
			Frame * frame;
			frame = this->tun.get_frame();
			frame->copy(from_app);
			int frame_size = frame->total_length();
			this->current_frame = *frame;
			if((((this->framework.send(from_app,frame_size,0,this->sequenceN))))){
				cout << "INFO: Dado com sequência " << this->sequenceN << " Enviado com sucesso!" << endl;
				this->currentstate = B;
			}else{
				cout << "ERRO: Erro ao enviar dado com sequência " << this->sequenceN << endl;
				return true;
			}
			delete from_app;
		}else if(this->received){						// Recebeu um quadro do transceiver
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = A;
				}else{
					cout << "ERRO: Tipo de dado não coerente: A!" << endl;
					return true;
				}
			}else{
				return true;
			}
			delete from_serial;
		}else{
			cout << "AVISO: Operação do estado A inválido" << endl;
			return true;
		}

		break;

	case B:

		cout << "INFO: Estado ARQ: B " << endl;
		this->enable = false;

		if(this->received){								// Recebeu um quadro do transceiver
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = B;
				}else if(test_ack(r)){
					srand((unsigned)time(0)); //para gerar números aleatórios reais.
					int maior = 5;
					int menor = 1;
					int aleatorio = rand()%(maior-menor+1) + menor;
					this->enable = true;
					this->backoff_value = aleatorio;
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado ou confirmação não coerente: B!" << endl;
					return true;
				}
			}
			delete from_serial;
		}else if(this->timeout){
			srand((unsigned)time(0)); //para gerar números aleatórios reais.
			int maior = 5;
			int menor = 1;
			int aleatorio = rand()%(maior-menor+1) + menor;
			this->enable = true;
			this->backoff_value = aleatorio;
			this->currentstate = D;
		}else{
			cout << "AVISO: Operação do estado B inválido" << endl;
			return true;
		}

		break;

	case C:

		cout << "INFO: Estado ARQ: C " << endl;

		if(this->backoff){
			this->enable = false;
			this->currentstate = A;
		}else if(this->received){
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado não coerente: C!" << endl;
					return true;
				}
			}
			delete from_serial;
		}else if(this->canSend){
			char * from_app = new char[BUFSIZE];
			Frame * frame;
			frame = this->tun.get_frame();
			frame->copy(from_app);
			cout << "AVISO: Dado não pode ser processado - Backoff!";
			return true;
		}else{
			cout << "AVISO: Operação do estado C inválido" << endl;
			return true;
		}

		break;

	case D:

		cout << "INFO: Estado ARQ: D " << endl;

		if(this->backoff){

			cout << "INFO: Retransmissão" << endl;
			char * from_app = new char[BUFSIZE];
			this->current_frame.copy(from_app);
			int frame_size = this->current_frame.total_length();
			if((((this->framework.send(from_app,frame_size,0,this->sequenceN))))){
				cout << "INFO: Dado com sequência " << this->sequenceN << " Enviado com sucesso!" << endl;
				this->currentstate = B;
			}else{
				cout << "ERRO: Erro ao enviar dado com sequência " << this->sequenceN << endl;
				return true;
			}
		}else if(this->received){
			Framework::Type r;
			char * from_serial = new char[FRAME_MAXSIZE];
			if((r = this->framework.receive(from_serial)) != Framework::none){
				if(test_data(r)){
					this->tun.write(from_serial, this->framework.get_len_receive());
					this->currentstate = D;
				}else if(test_ack(r)){
					srand((unsigned)time(0)); //para gerar números aleatórios reais.
					int maior = 5;
					int menor = 1;
					int aleatorio = rand()%(maior-menor+1) + menor;
					this->enable = true;
					this->backoff_value = aleatorio;
					this->currentstate = C;
				}else{
					cout << "ERRO: Tipo de dado não coerente: D!" << endl;
					return true;
				}
			}
			delete from_serial;
		}else if(this->canSend){
			char * from_app = new char[BUFSIZE];
			Frame * frame;
			frame = this->tun.get_frame();
			frame->copy(from_app);
			cout << "AVISO: Dado não pode ser processado - Backoff!";
			return true;
		}else{
			cout << "AVISO: Operação do estado D inválido" << endl;
			return true;
		}

		break;

	}
	return false; // Não ocorreu problema
}

/*
 * Esse método verifica da origem do dado recebido para
 * a transmissão do ACK(0|1) correspondente.
 *
 *    ** Verifica se o dado recebido é data0 | data 1:
 *	     => data0 : ACK sequência 0.
 *         
 *           => data1 : ACK sequência 1.
 * 
 */
bool ARQ::test_data(Framework::Type r){

	if(r == Framework::data0){
		this->sequenceM = 0;
		if((this->framework.send((char *)"a",1,1,this->sequenceM) > 0)){
			cout << "INFO: ACK com sequência " << this->sequenceM << " Enviado com sucesso!" << endl;
			return true;
		}else{
			cout << "ERRO: Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
			return false;
		}
	}else if(r == Framework::data1){
		this->sequenceM = 1;
		if((this->framework.send((char *)"a",1,1,this->sequenceM) > 0)){
			cout << "INFO: ACK com sequência " << this->sequenceM << "Enviado com sucesso!" << endl;
			return true;
		}else{
			cout << "ERRO: Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
			return false;
		}
	}
	return false;
}

/*
 * Esse método verifica se o ACK (0|1) foi
 * recebido na sequência correta.
 *
 *    ** Verifica se o dado recebido é data0 | data 1:
 *	     => ack0 : ACK sequência 0.
 *         
 *           => ack1 : ACK sequência 1.
 *
 *    ** Caso tenha ocorrido algum erro no recebimento 
 *       do ACK (0|1), a FSM do ARQ tratará isso através
 *       da retransmissão do ACK.
 */
bool ARQ::test_ack(Framework::Type r){

	if(r == Framework::ack0){
		if(sequenceN == 0){
			cout << "INFO: ACK recebido de sequência: " << this->sequenceN << endl;
			sequenceN = 1;
			return true;
		}else{
			cout << "ERRO: Erro ao enviar ACK de sequência: " << this->sequenceM << endl;
		}
	}else if(r == Framework::ack1){
		if(sequenceN == 1){
			cout << "INFO: ACK recebido de sequência: " << this->sequenceN << endl;
			sequenceN = 0;
			return true;
		}
	}
	return false;
}


