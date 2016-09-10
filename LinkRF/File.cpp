/*
 * File.cpp
 *
 *  Created on: 8 de set de 2016
 *      Author: mathias
 */

#include "File.h"

File::File(char *path){

	this->path = path;

	this->size = getSize();

	std::cout << "File size: " << this->size << std::endl;
}

File::~File(){}


/*
 * int File::readfile(char * buffer, unsigned int len, unsigned long position)
 *
 * char* buffer - It is the buffer where will placed the data from file.
 * unsigned in len - It is the size of the buffer.
 * unsigned long position - It is the position of cursor in reading the file.
 *
 * This method read the data of a file in the specific position and in the amount
 * of len to the buffer.
 *
 */
int File::readfile(char * buffer, unsigned int len, unsigned long position){

	std::ifstream in_file (this->path, std::ios_base::in | std::ios_base::binary );
	if(!in_file.is_open()){
		std::cerr << "Fail to open the file in write mode!" << std::endl;
		return -1;
	}else{
		in_file.seekg(position,std::ios_base::beg); // changes the cursor's position
		if(!(in_file.read(buffer,len))){
			std::cerr << "Fail to read from file!\n" << std::endl;
			return -1;
		}
	}
	in_file.close();
	return  1;
}

/*
 * int File::writefile(char *buffer);
 *
 * char *buffer - It is the data that must be appended in the file.
 *
 * This method read the data from buffer to the end of the specific file.
 */

int File::writefile(char *buffer){
	std::ofstream out_file (this->path, std::ios_base::out | std::ios_base::app | std::ios_base::binary );
	if(!out_file.is_open()){
		std::cerr << "Fail to open the file in write mode!" << std::endl;
		return -1;
	}else{
		if(!(out_file.write(buffer,strlen(buffer)))){
			std::cerr << "Fail to read from file!\n" << std::endl;
			return -1;
		}
	}
	out_file.close();
	return  1;
}

/*
 * long File::getSize(){
 *
 * This function return the size of the specific file.
 */


long File::getSize(){

	struct stat inf_file;

	if((stat(this->path, &inf_file)) == -1){
		std::cout << "Fail to get file information " << std::endl;
		return -1;
	}else{
		return inf_file.st_size;
	}
}
