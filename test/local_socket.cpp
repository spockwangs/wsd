// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv)
{
    cout << "here" << endl;
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    cout << "here" << endl;
    if (fd < 0) {
        cerr << "socket() failed" << endl;
        return -1;
    }

    struct sockaddr_un un;
    memset(&un, 0, sizeof(un));
    un.sun_family = AF_UNIX;
    cout << "here" << endl;
    strcpy(un.sun_path, "a.sock");
    int len = offsetof(struct sockaddr_un, sun_path) + strlen("a.sock");
    cout << "len=" << len << endl;
    int ret = ::bind(fd, (struct sockaddr*)&un, len);
    if (ret < 0) {
        cerr << "bind() failed" << endl;
        return -1;
    }

    pause();
    return 0;
}

