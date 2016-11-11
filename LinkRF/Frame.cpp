#include <string.h>
#include <arpa/inet.h>
#include "Frame.h"

#define Frame_TypeField 0x1
#define Frame_SeqField 0x2

Frame::Frame(char * buffer, int bytes) {
    if (bytes > MTU+4) throw -10;
    type = (FrameType)(buffer[0] & Frame_TypeField);
    seq = (buffer[0] & Frame_SeqField) >> 1;
    proto = buffer[1];
    len = bytes-4; // descontar o CRC e o cabeçalho
    memcpy(payload, buffer+2, len);
}

void Frame::copy(char * buffer) {
    buffer[0] = (type | (seq << 1)) & 0xff;
    buffer[1] = proto & 0xff;    
    if (len) memcpy(buffer+2, payload, len);    
}

Frame & Frame::operator=(const Frame & outro) {
    type = outro.type;
    seq = outro.seq;
    proto = outro.proto;
    len = outro.len;
    if (len) memcpy(payload, outro.payload, len);    
    return *this;    
}

void Frame::set_proto(unsigned short p) {
   switch (p) {
       case 0x0800:
           proto = 1; // IPv4
           break;
       case 0x866d:
           proto = 2; // IPv6
           break;
       default:
           throw -1; // desconhecido !!!
   }        
}


unsigned short Frame::get_proto() const {
    unsigned short p;
    
   switch (proto) {
       case 1:
           p = 0x800; // IPv4
           break;
       case 2:
           p = 0x866d; // IPv6
           break;
       default:
           throw -1; // desconhecido !
   }
   
   return p;
}

void Frame::copy_payload(char* buffer) const {
    memcpy(buffer, payload, len);
}

ostream & operator<<(ostream & out, const Frame * f) {
    out << "Frame: seq=" << f->seq;
    out << ", type=" << f->type;
    out << ", proto=" << f->proto;
    out << ", len=" << f->len;
    return out;
}

