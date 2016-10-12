#ifndef SERIAL_H
#define SERIAL_H

#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>

//#define BUFSIZE 1024
#define MAX_SERIAL_BYTES 64

// Serial: uma classe para encapsular dispositivos seriais

class Serial {
 protected:
  int tty_fd;
 public:
  // construtor: path=pathname da porta serial (ex: "/dev/ttyUSB0"),
  // rate=taxa de bits (usar as constantes do termios, ex: B9600)
  Serial(const char * path, int rate);

  ~Serial();

  // escreve até "size" bytes na porta serial, a partir a área de memória apontada
  // por "buffer"
  // Retorna a quantidade de bytes efetivamente escritos
  int write(char * buffer, int len);


  // lê até "size" bytes da porta serial, guardando-os na área de memória apontada
  // por "buffer". Bloqueia até que ao menos 1 byte seja recebido.
  // Retorna a quantidade de bytes efetivamente lidos
  int read(char * buffer, int len);

  // lê até "size" bytes da porta serial, guardando-os na área de memória apontada
  // por "buffer". Se "block" for true, bloqueia até que ao menos 1 bytes seja recebido.
  // Se "block" for false, retorna imediatamente, mesmo que nenhum byte tenha sido lido.
  // Retorna a quantidade de bytes efetivamente lidos
  int read(char * buffer, int len, bool block);

  char read_byte();
};

#endif
