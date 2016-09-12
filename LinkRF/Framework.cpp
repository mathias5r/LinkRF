/*
 * Framming.cpp
 *
 *  Created on: 25 de ago de 2016
 *      Authors: Mathias Silva da Rosa
 *		 Vinicius Bandeira
 *		 Guilherme de Albuquerque
 */

#include "Framework.h"

//using namespace std;

// This vector is used to calculate the CRC of the frames.
static uint16_t fcstab[256] = {
			0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
			0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
			0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
			0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
			0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
			0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
			0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
			0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
			0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
			0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
			0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
			0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
			0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
			0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
			0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
			0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
			0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
			0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
			0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
			0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
			0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
			0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
			0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
			0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
			0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
			0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
			0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
			0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
			0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
			0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
			0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
			0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


Framework::Framework(Serial & s, int bytes_min, int bytes_max):serial(s) {
	this->min_bytes = bytes_min;
	this->max_bytes = bytes_max;
	this->buffer = new char[BUFSIZE];
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
 * This function delimitates the data with flags 0x7E, replaces false flags,
 * places CRC bytes and sends the final frame through the serial port.
 */
void Framework::send(char *buffer, int len){

	/*It is multiplied by 2 because all of data could be
	  false flags and it is added by 5 because of flags,
	  CRC and char terminator '\0';
	*/
	char frame[2*BUFSIZE+5];

	std::cout << "Framework started" << std::endl;

	if (len >= this->min_bytes && len <= this->max_bytes) {

		frame[0] = 0x7E; // Initial delimitation of the frame using flag 0x7E (01111110)

		int i,j;
		for( i=0, j=1 ; i <= len ; i++,j++ ){
			switch(buffer[i]){
				case(0x7E):	 // If there is a 0x7E byte in the data, it is placed 0x7D5E in frame
					frame[j] = 0x7D;
					frame[j+1] = 0x5E;
					j = j + 1;
					std::cout << "INFO: 0x7E found in position "<< i << " of the data." << std::endl;
					break;
				case(0x7D):	// If there is a 0x7D byte in the data, it is placed 0x7D5D in frame
					frame[j] = 0x7D;
					frame[j+1] = 0x5D;
					j = j + 1;
					std::cout << "INFO: 0x7D found in position "<< i << " of the data." << std::endl;
					break;
				default:
					frame[j] = buffer[i]; // Normal copy
			}
		}

		gen_crc((unsigned char *)frame,j-1); //CRC generation

		frame[j+1] = 0x7E; // End delimitation of the frame using flag 0x7E (01111110)

		frame[j+2] = '\0'; // Char delimitator - The RF receiver needs it to recognize a new frame

		std::cout << "Final Frame: " << frame << std::endl;

		int n;
		if(!(n = serial.write(frame, strlen(frame))) > 0){
			std::cout << "Error in writing a frame in the serial port" << n << std::endl;
		}

	} else {
		std::cout << "ERROR: Frame exceeded the maximum/minimum size" << std::endl;
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

	std::cout << "Removal of framework started" << std::endl;

	bool return_fsm;
	char frame[2*BUFSIZE+4];
	char frame_byte;

	this->serial.read(frame, BUFSIZE, false);
	std::cout << "Frame read: " << frame << std::endl;

	for(int i=0;!return_fsm; i++){
		frame_byte = frame[i];
		return_fsm = this->handle(frame_byte);
	}

	if(check_crc((unsigned char*)this->buffer,n_bytes)){
		memcpy(buffer, this->buffer, BUFSIZE);
		std::cout << "Data received: " << buffer  << std::endl;
	}else{
		std::cout << "CRC does not match!: " << buffer  << std::endl;
	}

	return this->n_bytes;
}

/*
 * bool Framework::handle(char byte);
 *
 * char byte - It is the byte of the frame that must be analyzed
 *
 * This method implements the FSM of the receiver
 *
 */
bool Framework::handle(char byte){

	switch(this->currentState){

	case waiting:
		if(byte == 0x7E){
			std::cout << "Initial delimitation found!" << std::endl;
			this->n_bytes = 0;
			this->currentState = reception;
		} else {
			this->currentState = waiting;
		}
		break;

	case reception:
		if (this->n_bytes > this->max_bytes){
			std::cout << "Overflow: " << this->n_bytes << " bytes" << std::endl;
			this->currentState = waiting;
		} else {
			if(byte == 0x7E){
				std::cout << "End delimitation found!" << std::endl;
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
			std::cout << "INFO: 0x7E found in position: " << n_bytes << " of the data" << std::endl;
    		this->buffer[n_bytes] = 0x7E;
			this->currentState = reception;
			break;
		case(0x5D):
			std::cout << "INFO: 0x7D found in position: " << n_bytes << " of the data" << std::endl;
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

/*
 * ool Framework::check_crc(unsigned char * buffer, int len);
 *
 * unsigned char * buffer - It is the frame that has the CRC.
 * int len - It is the size of the buffer
 *
 * This method check if the CRC in the end of the frame corresponds
 * with the data received
 *
 */
bool Framework::check_crc(unsigned char * buffer, int len){

	uint16_t crc = pppfcs16(buffer,len);

	return (crc == PPPGOODFSCS16);
}


/*
 * void Framework::gen_crc(unsigned char * buffer, int len);
 *
 * unsigned char * buffer - The CRC must be placed in the end of this buffer
 * int len - It is the size of the buffer
 *
 * This method generates the CRC analyzing the data into the buffer and
 * places the CRC in the end of this. The CRC is calculated by the frame
 * check sequence method.
 *
 */
void Framework::gen_crc(unsigned char * buffer, int len){

	unsigned char * buffer_copy = new unsigned char[2*BUFSIZE];
	memcpy(buffer_copy,buffer+1,len-1);

	uint16_t crc = pppfcs16(buffer_copy,len);

	crc ^= 0xffff;
	uint8_t crc_low = (uint8_t)crc;
	std::cout << "INFO: Calculated low crc: " << crc_low << std::endl;

	uint8_t crc_high = (uint8_t)(crc >> 8);
	std::cout << "INFO: Calculated high crc: " << crc_high << std::endl;

	buffer[len] = (unsigned char)crc_high & 0xff;
	buffer[len+1] = (unsigned char)crc_low & 0xff;

	delete[] buffer_copy;
}


uint16_t Framework::pppfcs16(unsigned char * cp, int len){

	uint16_t fcs = PPPINITFCS16;

	while(len--){
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];
	}
	return(fcs);
}



