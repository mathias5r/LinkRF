/* 
 * File:   Frame.h
 * Author: msobral
 *
 * Created on 15 de Março de 2016, 14:22
 */

#ifndef FRAME_H
#define	FRAME_H

#include <fstream>

using namespace std;

#include "Definitions.h"

enum FrameType { Data, ACK};

struct Frame {
    FrameType type;
    int seq;
    int proto;
    char payload[MTU];
    int len;
    int tx; // transmissoes
    
    Frame() {}
    Frame(const Frame & outro) {*this = outro;}
    Frame &operator=(const Frame & outro);
    Frame (char * buffer, int bytes);
    void copy(char * buffer);
    int total_length() const { return len+4;}
    void set_proto(unsigned short p);
    unsigned short get_proto() const;
    void copy_payload(char * buffer) const;
};

ostream & operator<<(ostream & out, const Frame * f);

#endif	/* FRAME_H */

