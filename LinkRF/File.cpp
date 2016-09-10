/*
 * File.cpp
 *
 *  Created on: 8 de set de 2016
 *      Author: mathias
 */

#include "File.h"

File::File(char *path){

	this->path = path;

	std::fstream f (this->path, std::ios_base::out | std::ios_base::in | std::ios_base::binary );
	if(!f){std::cerr << "Fail to open the file!" << std::endl;}

	this->file = &f;

	this->size = getSize();

	std::cout << "File size: " << this->size << std::endl;
}

File::~File(){}

int File::readfile(char * buffer, unsigned long position){
	if(!(this->file->read(buffer,this->size))){
		std::cerr << "Fail to read from file!\n" << std::endl;
		return -1;
	}else{
		this->file->seekg(position);
		return 1;
	}
}

int File::writefile(char *buffer, unsigned long position){
	if(!(this->file->write(buffer,this->size))){
		std::cerr << "Fail to read from file!\n" << std::endl;
		return -1;
	}else{
		this->file->seekp(position);
		return 1;
	}
}


long File::getSize(){

	struct stat inf_file;

	if((stat(this->path, &inf_file)) == -1){
		std::cout << "Fail to get file information " << std::endl;
		return -1;
	}else{
		return inf_file.st_size;
	}
}
