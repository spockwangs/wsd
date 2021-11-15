// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd/benchmark.h"

int main(int argc, char** argv)
{
    wsd::benchmark::Init(&argc, &argv);
    return wsd::benchmark::RunTests();
}
