// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __CHECKED_DELETE_H__
#define __CHECKED_DELETE_H__

namespace wsd {

    // verify that types are complete when deleted
    
    template <typename T>
    inline void checked_delete(T *p)
    {
        typedef char type_must_be_complelete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complelete);
        delete p;
    }

    template <typename T>
    inline void checked_array_delete(T *p)
    {
        typedef char type_must_be_complelete[sizeof(T) ? 1 : -1];
        (void) sizeof(type_must_be_complelete);
        delete[] p;
    }

}  // namespace wsd

#endif  // __CHECKED_DELETE_H__
