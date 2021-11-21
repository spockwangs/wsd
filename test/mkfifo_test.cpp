// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>

using namespace std;

void catcher(int)
{
}

int main(int argc, char** argv)
{
    struct sigaction sigact;

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = catcher;
    sigaction(SIGTERM, &sigact, NULL);

    int ret = mkfifo("/tmp/fifo", 0644);
    if (ret && errno != EEXIST) {
        cerr << "mkfifo() failed: " << strerror(errno) << endl;
        return -1;
    }
    
    FILE* fout = fopen("/tmp/fifo", "w");
    if (fout == nullptr) {
        cerr << "fopen() failed: " << strerror(errno) << endl;
        return -1;
    }

    char a = 'a';
    ret = fwrite(&a, sizeof(a), 1, fout);
    a = '\n';
    ret = fwrite(&a, sizeof(a), 1, fout);
    cout << "ret=" << ret << endl;
    ret = fflush(fout);
    if (ret) {
        cerr << ret << endl;
    }
    pause();

    ret = fwrite(&a, sizeof(a), 1, fout);
    cout << "ret=" << ret << endl;
    ret = fflush(fout);
    if (ret) {
        cerr << ret << strerror(errno) << endl;
    }
}

