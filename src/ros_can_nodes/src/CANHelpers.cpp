/*
 * Date Started: 3/12/2017
 * Original Author: Simon Ireland
 * Editors:
 * ROS Node Name:
 * ROS Package: ros_can_nodes
 * Purpose:
 * This code is released under the MIT  License. Copyright BLUEsat UNSW, 2017
 */


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "CANHelpers.hpp"

int soc;

int CANHelpers::open_can_port(const char *const port) {
    ifreq ifr;
    sockaddr_can addr;

    /* open socket */
    soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (soc < 0) {
        return (-1);
    }

    addr.can_family = AF_CAN;
    strcpy(ifr.ifr_name, port);

    if (ioctl(soc, SIOCGIFINDEX, &ifr) < 0) {
        return (-1);
    }

    addr.can_ifindex = ifr.ifr_ifindex;

    fcntl(soc, F_SETFL, O_NONBLOCK);

    if (bind(soc, (sockaddr *)&addr, sizeof(addr)) < 0) {
        return (-1);
    }

    return 0;
}

int CANHelpers::send_can_port(const can_frame& frame) {
    const int retval = write(soc, &frame, sizeof(can_frame));
    if (retval != sizeof(can_frame)) {
        printf("Failed to send !! %s \n", strerror(errno));
        return (-1);
    } else {
        printf("Sent !!\n");
        return (0);
    }
}

int CANHelpers::read_can_port(can_frame& frame) {
    int recvbytes = -1;

    // 1 second timeout on read, will adjust based on testing
    timeval timeout = {1, 0};
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(soc, &readSet);

    // Check if the socket is ready to read from
    // Possibly check errno for bad FD
    if (select((soc + 1), &readSet, NULL, NULL, &timeout) >= 0) {
        if (FD_ISSET(soc, &readSet)) {
            recvbytes = read(soc, &frame, sizeof(can_frame));
        }
    }

    return recvbytes;
}

void CANHelpers::close_can_port() {
    close(soc);
}
