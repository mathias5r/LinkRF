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
#include <sys/stat.h>

#define BUFSIZE 1024

class File {
public:
	File(FILE* file, char *path);
	virtual ~File();
	int readfile(char * buffer);
	long writefile(char * buffer);
	long getSize();
private:
	FILE * file;
	char * path;
	long size;

};

#endif /* FILE_H_ */
