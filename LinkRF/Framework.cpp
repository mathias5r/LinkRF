/*
 * Framming.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Author: vinicius
 */

#include "Framework.h"

using namespace std;

Framework::Framework(Serial & s, int bytes_min, int bytes_max):serial(s) {
	this->min_bytes = bytes_min;
	this->max_bytes = bytes_max;
	this->buffer[BUFSIZE];
	this->n_bytes = 0;
	this->currentState = waiting;
}

Framework::~Framework() {}

/*
 * void Framming::send(char *buffer, int len);
 *
 * char *buffer - It is the data that must be placed in a frame.
 * int len - The size of the data in bytes
 *
 * This function delimitate the data with flags 0x7E, replace false flags
 * and send the frame through the serial port.
 */
void Framework::send(char *buffer, int len){

	char frame[2*BUFSIZE+2];

	cout << "Framework started" << endl;

	if (len >= this->min_bytes && len <= this->max_bytes) {

		frame[0] = 0x7E;							// Initial delimitation of the frame using flag 0x7E (01111110)
		int i,j;
		for( i=0, j=1 ; i <= len ; i++,j++ ){
			switch(buffer[i]){
			case(0x7E):							// If there is a 0x7E byte in the data, it is placed 0x7D5E in frame
														frame[j] = 0x7D;
			frame[j+1] = 0x5E;
			j = j + 1;
			cout << "INFO: 0x7E founded in position "<< i << " of the data." << endl;
			break;
			case(0x7D):							// If there is a 0x7D byte in the data, it is placed 0x7D5D in frame
														frame[j] = 0x7D;
			frame[j+1] = 0x5D;
			j = j + 1;
			cout << "INFO: 0x7D founded in position "<< i << " of the data." << endl;
			break;
			default:
				frame[j] = buffer[i];			// Normal copy
			}
		}
		frame[j-1] = 0x7E;							// End delimitation of the frame using flag 0x7E (01111110)
		frame[j] = '\n';

		int n = serial.write(frame, strlen(frame));

		if(! n > 0){
			std::cout << "Error in writing a frame in the serial port" << n << std::endl;
		}
	} else {
		cout << "ERROR: Frame exceeded the maximum/minimum size" << endl;
	}
}

/*
 * char* Framming::receive(char* buffer);
 *
 * char* frame - It is the buffer where is placed the data
 *
 * This function receive the frame from serial port, extract
 * the date from the frame ignoring the flags and false flags
 * and put it in the buffer.
 *
 */
int Framework::receive(char* buffer){

	cout << "Removal of framework started" << endl;

	bool return_fsm;
	char frame_byte;

	for(int i=0;!return_fsm; i++){
		this->serial.read(buffer, BUFSIZE, false);
		frame_byte = buffer[i];
		return_fsm = this->handle(frame_byte);
	}

	cout << "Data received: " << buffer  << endl;

	return this->n_bytes;
}


bool Framework::handle(char byte){


	switch(this->currentState){

	case waiting:
		if(byte == 0x7E){
			this->n_bytes = 0;
			this->currentState = reception;
		} else {
			this->currentState = waiting;
		}
		break;

	case reception:
		if (this->n_bytes > this->max_bytes){
			cout << "Overflow: " << this->n_bytes << " bytes" << endl;
			this->currentState = waiting;
		} else {
			if(byte == 0x7E){
				this->currentState = waiting;
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
    		this->buffer[n_bytes] = 0x7E;
			this->currentState = reception;
			break;
		case(0x5D):
			this->buffer[n_bytes] = 0x7D;
			this->currentState = reception;
			break;
		default:
			this->buffer[n_bytes] = byte;
			this->currentState = reception;
			break;
		}
		break;
	}
	return false;
}
