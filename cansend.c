/*
 *  $Id$
 */

/*
 * cansend.c - simple command line tool to send and receive
 * 		CAN-frames via CAN_RAW sockets
 *
 * Copyright (c) 2002-2007 Volkswagen Group Electronic Research
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Volkswagen nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * Alternatively, provided that this notice is retained in full, this
 * software may be distributed under the terms of the GNU General
 * Public License ("GPL") version 2, in which case the provisions of the
 * GPL apply INSTEAD OF those given above.
 *
 * The provided data structures and external interfaces from this code
 * are not restricted to be used by modules with a GPL compatible license.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Send feedback to <socketcan-users@lists.berlios.de>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include "can.h"
#include "raw.h"

#include "lib.h"

#define SIOCSCANBAUDRATE	0x89F0

int main(int argc, char **argv)
{
    int s; /* can raw socket */ 
    int nbytes, ret;
    struct sockaddr_can addr;
    struct can_frame frame, rframe;
    struct ifreq ifr;
    int i;

    /* check command line options */
    if ((argc != 2) && (argc != 3)) {
        fprintf(stderr, "Usage: send frame: %s <device>.\n", argv[0]);
	fprintf(stderr, "Usage: receive frame: %s <device> <can_frame>.\n", argv[0]);
	return 1;
    }

    /* open socket */
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
      perror("socket");
      return 1;
    }

    strcpy(ifr.ifr_name, argv[1]);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    ifr.ifr_ifru.ifru_ivalue = 1000000/2;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("bind");
      return 1;
    }

    if (argc ==3) {
	    /* parse CAN frame */
	    if (parse_canframe(argv[2], &frame)){
		fprintf(stderr, "\nWrong CAN-frame format!\n\n");
		fprintf(stderr, "Try: <can_id>#{R|data}\n");
		fprintf(stderr, "can_id can have 3 (SFF) or 8 (EFF) hex chars\n");
		fprintf(stderr, "data has 0 to 8 hex-values that can (optionally)");
		fprintf(stderr, " be seperated by '.'\n\n");
		fprintf(stderr, "e.g. 5A1#11.2233.44556677.88 / 123#DEADBEEF / ");
		fprintf(stderr, "5AA# /\n     1F334455#1122334455667788 / 123#R ");
		fprintf(stderr, "for remote transmission request.\n\n");
		return 1;
	    }
	    if ((nbytes = write(s, &frame, sizeof(frame))) != sizeof(frame)) {
		    perror("write");
		    return 1;
	    }
    } else if (argc == 2) {
	    if ((nbytes = read(s, &rframe, sizeof(rframe))) < 0) {
		    perror("rx frame read");
	    	    return 0;
	    }
            printf("\nread %d bytes\n", nbytes);
	    fprint_long_canframe(stdout, &rframe, "\n", 0);
    }

    close(s);
    return 0;
}
