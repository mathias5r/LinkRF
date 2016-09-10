/*
 * File.cpp
 *
 *  Created on: 8 de set de 2016
 *      Author: mathias
 */

#include "File.h"

File::File(FILE* file, char *path){

	this->file = file;
	this->path = path;

	if((this->file = fopen(this->path, "wa")) == NULL) { //Abre arquivo com mesmo nome em modo escrita
		std::cout << "Fail to open the file!\n" << std::endl;
	}

	this->size = getSize();
}

File::~File(){}

int File::readfile(char * buffer){
	if(this->file != NULL){
		fread(buffer,this->size,1024,this->file);
		buffer[this->size] = 0;
		return 1;
	}else{
		return -1;
	}
}

long File::writefile(char *buffer){

	int b_size = strlen(buffer);

	if(this->file != NULL){
		while(b_size != 0){
			fwrite(buffer,sizeof(char),1024,file); // Recebe 1kb por vez e grava
			b_size -= 1024;
		}
		return 1;
	}
	return -1;
}


long File::getSize(){

	struct stat inf_file;
	int f_size;

	if((f_size = stat(this->path, &inf_file)) != -1){
		std::cout << "Fail to get file information " << std::endl;
	}
}
