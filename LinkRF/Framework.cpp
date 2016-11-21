/*
 * Framming.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Authors: Mathias Silva da Rosa
 *		 Vinicius Bandeira
 *		 Guilherme de Albuquerque
 */

#include "Framework.h"
#include <bitset>

/*
 * Construtor da classe Framework.
 * 	=> Inicializou as variáveis para construção do quadro que será transmitido
 *         através da interface serial;
 *      => O objeto CRC para adicionar o FCS (Frame Check Sequence) no pacote
 *         que será transmitido;
 *      
 */ 
Framework::Framework(Serial & tr, int bytes_min, int bytes_max): transceiver(tr){

	this->min_bytes = bytes_min;
	this->max_bytes = bytes_max;
	this->buffer = new char[BUFSIZE];
	this->n_bytes = 0;
	this->currentState = waiting;
	CRC crc;
	this->crc = &crc;
	this->len_send = 0;
	this->len_receive = 0;
}

/*
 * Esse método é utilizado na recepção do quadro.
 * Basicamente, verifica se o quadro está dentro do tamanho mínimo e
 * máximo de bytes e verifica se é possível enviar o quadro ao transceiver.
 * 
 * ** Verifica se o quadro está dentro do tamanho mín e max estabelecido. 
 *	=> Caso sim, tenta transmitir o quadro através do transceiver.
 *      => Caso não, é transmitido o erro de tamanho inválido de quadro
 *	   e o quadro é descartado.
 *
 * ** Verifica se é possível enviar o quadro através do transceiver. Ou seja,
 *    se é possível a escrita no descritor do transceiver.
 * 	=> Caso sim, o quadro é transmitido com sucesso.
 *	=> Caso não, o quadro é descartado.
 *
 */
int Framework::send(char * buffer, int len, int type, int seq){

	cout << "INFO: Início do enquadramento..." << endl;

	char* frame = new char[FRAME_MAXSIZE];

	if (len >= this->min_bytes && len <= this->max_bytes) {
		frame = mount(buffer,len,type, seq);
		int k;
		if((k = transceiver.write(frame, this->len_send) > 0)){
			len_send = 0;
			cout << "INFO: Escrita no descritor do transceiver: " << this->transceiver.get_fd() << " com sucesso!" << endl;
		}else{
			cout << "ERRO: Falha na escrita no descritor do transceiver: " << this->transceiver.get_fd() << endl;
			return -1;
		}
	} else {
		cout << "ERRO: Quadro excedeu o limite máximo de envio: " << len << endl;
		return -1;
	}

	delete frame;
	return 1;
}

/*
 * Esse método adiciona as flags de início e fim de quadro. 
 * Entretanto, durante a inserção das flags no quadro, também
 * é necessário verificar se as flags também não estão contidas
 * no payload do quadro. Caso estejam, é realizado um tratamento
 * no payload do quadro.
 *
 * ** Adiciona Flag de início do quadro (7E)
 * 
 * ** Verificar se é um quadro de ACK ou de dados.
 *	=> Através do retorno do byte control é possível
 *         saber se é um ACK (0|1) ou data (0|1)
 *
 * ** Iteração para buscar as flags no payload do quadro.
 * 	=> Byte do payload = 7E 
 *		*** Na mesma posição é adicionado 7D e na próxima 5E.
 *		    No momento da recepção (desmontrar o quadro), 
 *                  assim que for verificado o 7D, o 5E voltará 
 *                  a ser 7E.
 *
 *	=> Byte do payload = 7D
 *		*** Na mesma posição é adicionado 7D e na próxima 5D.
 *		    No momento da recepção (desmontrar o quadro), 
 *                  assim que for verificado o 7D, o 5D voltará 
 *                  a ser 7D.
 *
 *	=> Default
 *		*** O byte é só adicionado ao quadro.
 *
 *
 * ** Adiciona Flag de fim de quadro (7E)
 *
 */
char * Framework::mount(char * buffer, int len, int type, int seq){

	this->len_send = 0;

	char* frame =  new char[FRAME_MAXSIZE];

	frame[0] = 0x7E; // Flag de início de quadro

	frame[1] = header(type, seq);

	int i,j;
	for( i=0, j=2 ; i < len ; i++,j++ ){
		switch(buffer[i]){
			case(0x7E):
				frame[j] = 0x7D;
				frame[j+1] = 0x5E;
				j = j + 1;
				cout << "INFO: Flag 0x7E na posição "<< i << "do dado!" << endl;
				break;
			case(0x7D):
				frame[j] = 0x7D;
				frame[j+1] = 0x5D;
				j = j + 1;
				cout << "INFO: Flag 0x7D na posição "<< i << "do dado!" << endl;
				break;
			default:
				frame[j] = buffer[i];
		}
	}

	this->crc->gen_crc(frame+2,j-2);

	frame[j+2] = 0x7E; // Flag de fim de quadro
	frame[j+3] = 0; // Delimitador de char

	this->len_send = j+3;
	return frame;
}

/*
 * 
 * Esse método é utilizado na recepção do quadro.
 * Basicamente, recebe o quadro byte a byte até a 
 * delimitação de fim de quadro e envia o quadro
 * recebido à FSM (Finite State Machine) para que
 * seja enviando somente payload do quadro a app.
 *
 * ** Laço de iteração para recepção do quadro.
 * 	=> Execedeu o tamanho: quadro descartado.
 *   
 *      => Erro ao ler byte do transceiver: quadro descartado.
 *
 *	=> Default: continua recebendo os bytes do quadro 
 * 		    até a flag "end_flag" ser habilitada.
 *
 * ** Verifica o CRC do quadro.
 *	=> CRC alterado: quadro descartado.
 *
 *	=> CRC inalterado: envia à FSM de recepção.
 *
 * ** Laço de iteração para o tratamento do payload
 *    pela FSM de recepção. 
 *
 */
Framework::Type Framework::receive(char * buffer){

	cout << "INFO: Início da remoção do enquadramento..." << endl;

	this->len_receive = 0;

	char * frame = new char[FRAME_MAXSIZE];

	bool return_fsm = false;
	char frame_byte;
	int n = 0, k = 0, v = 0;

	bool end_flag  = false;
	while(!end_flag){
		if(k >= FRAME_MAXSIZE){
			cout << "ERRO: Quadro excedeu o limite máximo de recepção: " << k << endl;
			return Framework::none;
		}
		if(!(n = this->transceiver.read(frame+k,1,true)) > 0){
			cout << "ERRO: Erro ao ler byte do transceiver: " << n << std::endl;
			return Framework::none;
		}else{
			k += n;
			if(frame[k-1] == '~'){
				v++;
				if(v >= 2){
					end_flag = true;
				}
			}
		}
	}
	cout << "INFO: Leitura do descritor do transceiver: " << this->transceiver.get_fd() << " com sucesso!" << endl;

	frame[k] = 0;

	char * crc_buffer = new char [FRAME_MAXSIZE];
	memcpy(crc_buffer,frame+2,k-3);

	if(!(this->crc->check_crc(crc_buffer,k-3))){
		std::cout << "CRC does not match!: " << buffer  << std::endl;
		return Framework::none;
	}

	for(int i=0;!return_fsm; i++){
		frame_byte = frame[i];
		return_fsm = this->handle(frame_byte);
	}

	this->len_receive = k-5;

	Framework::Type r = get_type(this->buffer[0]);

	memcpy(buffer, this->buffer+1, this->len_receive);

	delete crc_buffer;
	delete frame;
	return r;
}

/*
 * Implementação da FSM (Finite State Machine) para recepção do quadro. 
 *
 * Caso waiting:
 *
 *	** Verifica se recebeu a flag de início de quadro (7E)	
 *		=> Segue ao estado de recepção (reception).
 *      ** Continua no estado waiting (idle) na espera da 
 * 	   flag de início de quadro.
 *
 * Caso reception:
 *
 *	** Caso o número de bytes recebidos seja maior que o	
 *         tamanho máximo do quadro.
 *         	=> Retorna ao estado waiting à espera de um novo
 *		   quadro.
 *	** Flag 7E encontrada (flag de fim de quadro).	
 *		=> Retorna ao estado waiting à espera de um novo
 *		   quadro.
 *	** Flag 7D encontrada (flag de escape)
 *		=> Segue para o estado escape para o tratamento 
 *		   dessa exceção.
 *	** Default
 *		=> Continua recebendo os bytes do payload.
 *
 * Caso escape:
 *
 *	** Flag 5E.
 *		=> Transforma o 5E em 7E e remove 7D.
 *	** Flag 5D.
 *		=> Transforma o 5D em 7D e remove o 7D.
 *	** Flag 7E (flag fim de quadro).
 *		=> Retorna ao esta do waiting à espera de
 *		   um novo quadro.
 *	** Default
 *		=> Continua recebendo os bytes do payload.
 *
 */
bool Framework::handle(char byte){

	switch(this->currentState){

	case waiting:
		if(byte == 0x7E){
			cout << "INFO: Flag de início de quadro encontrada!" << endl;
			this->n_bytes = 0;
			this->currentState = reception;
		} else {
			this->currentState = waiting;
		}
		break;
	case reception:
		if (this->n_bytes > this->max_bytes){
			this->currentState = waiting;
		} else {
			if(byte == 0x7E){
				std::cout << "INFO: Flag de fim de quadro encontrada!" << std::endl;
				this->currentState = waiting;
				this->buffer[n_bytes] = 0;
				return true; // frame finished
			} if (byte == 0x7D) {
				this->currentState = escape;
			} else {
				this->buffer[n_bytes] = byte;
				this->n_bytes++;
				this->currentState = reception;
			}
		}
		break;
	case escape:
		switch(byte){
		case(0x5E):
			std::cout << "INFO: 0x7E encontrado na posição: " << n_bytes << " do dado" << std::endl;
    		this->buffer[n_bytes] = 0x7E;
    		this->n_bytes++;
			this->currentState = reception;
			break;
		case(0x5D):
			std::cout << "INFO: 0x7D encontrado na posição: " << n_bytes << " do dado" << std::endl;
			this->buffer[n_bytes] = 0x7D;
			this->currentState = reception;
			break;
		case(0x7E):
				std::cout << "INFO: Flag de fim de quadro encontrada!" << std::endl;
				this->currentState = waiting;
				return true; // frame finished
		default:
			this->buffer[n_bytes] = byte;
			this->currentState = reception;
			break;
		}
		break;
	}
	return false;
}

/*
 *
 * Esse método retorna um byte denominado control, para
 * que na recepção através desse byte control seja possível
 * verificar se é um ACK ou data e qual é sua sequência (0|1).
 * É utilizado no método "mount" para adicionar qual tipo de
 * quadro e qual sequência será transmitido.
 * 
 */
char Framework::header(int type, int seq){

	char control;

	if(type == 0 && seq == 0){ // Tipo Dado, Sequência 0;
		control = '0';
	}

	if(type == 0 && seq == 1){ // Tipo Dado, Sequência 1;
		control = '1';
	}

	if(type == 1 && seq == 0){ // Tipo ACK, Sequência 0;
		control = '2';
	}

	if(type == 1 && seq == 1){ // Tipo ACK, Sequência 1;
		control = '3';
	}

	return control;

}

/*
 *
 * Esse método retorna qual é o tipo quadro, ACK(0|1) ou data(0|1), 
 * através do byte control passado como parâmetro.
 * É utilizado na recepção do quadro para verificar qual tipo de
 * quadro recebido. 
 *
 */
Framework::Type Framework::get_type(char control){

	switch(control){
		case('0'):
			return Framework::data0;
			cout << "INFO: Dado de sequência 0 reconhecido!" << endl;
			break;
		case('1'):
			return Framework::data1;
			cout << "INFO: Dado de sequência 1 reconhecido!" << endl;
			break;
		case('2'):
			return Framework::ack0;
			cout << "INFO: ACK de sequência 0 reconhecido!" << endl;
			break;
		case('3'):
			return Framework::ack1;
			cout << "INFO: ACK de sequência 1 reconhecido!" << endl;
			break;
		default:
			cout << "Erro: Tipo de mensagem não identificada!: " << control << endl;
			break;
	}
	return Framework::none;
}


