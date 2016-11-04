/*
 * File:   Tun.h
 * Author: msobral
 *
 * Created on 9 de Março de 2016, 13:15
 */

#ifndef TUN_H
#define	TUN_H

#include <net/if.h>
#include "Definitions.h"
#include "Frame.h"

struct TunFrame {
    char buffer[MTU+4];
    int len;

    TunFrame() : len(0) {}
    TunFrame(int fd);
    TunFrame(Frame * f);
    operator Frame() const;
    unsigned short get_proto() const;
    void set_proto(unsigned short p);
};

class Tun {
public:
    Tun();
    Tun(const char * name, const char * ip, const char * dstip);
    Tun(const Tun& orig);
    virtual ~Tun();
    void start();
    int get();
    Frame * get_frame();
    void send_frame(Frame * f);
    int write(char * buffer, int len);
private:
    int fd;
    char dev[IFNAMSIZ], ip[16], dstip[16];

    void alloc();
    void setip();
};

#endif	/* TUN_H */
