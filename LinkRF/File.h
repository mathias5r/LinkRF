/*
 * File.h
 *
 *  Created on: 8 de set de 2016
 *      Author: mathias
 */

#ifndef FILE_H_
#define FILE_H_

#include<stdio.h>
#include<string.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<iostream>
#include<sys/stat.h>
#include<cstdlib>
#include <fstream>

#define BUFSIZE 1024

class File {
public:
	File(char *path);
	virtual ~File();
	int readfile(char * buffer, unsigned long position);
	int writefile(char * buffer, unsigned long position);
	long getSize();
private:
	std::fstream* file;
	char * path;
	long size;

};

#endif /* FILE_H_ */
