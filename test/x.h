// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __X_H__
#define __X_H__

#include <memory>
#include "scoped_ptr.h"

class X;

struct Pimpl {
Pimpl() {}

~Pimpl();
    
wsd::ScopedPtr<X> p;
};

#endif  // __X_H__
