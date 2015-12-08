// Str v0.11
// Simple c++ string type with an optional local buffer
// https://github.com/ocornut/str

// LICENSE
// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.

#include "Str.h"
#include <stdarg.h>         // va_list

// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to 2013 doesn't have it.
#ifndef va_copy
#define va_copy(dest, src) (dest = src)
#endif

// Clear 
void    Str::clear()
{
    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);
    if (LocalBufSize)
    {
        Data = local_buf();
        Data[0] = '\0';
        Capacity = LocalBufSize;
        Owned = 1;
    }
    else
    {
        Data = "";
        Capacity = 0;
        Owned = 0;
    }
}

// Reserve memory, preserving the current of the buffer
void    Str::reserve(int new_capacity)
{
    if (new_capacity <= Capacity)
        return;

    char* new_data;
    if (new_capacity < LocalBufSize)
    {
        // Disowned -> LocalBuf
        new_data = local_buf();
        new_capacity = LocalBufSize;
    }
    else
    {
        // Disowned or LocalBuf -> Heap
        new_data = (char*)STR_MEMALLOC(new_capacity * sizeof(char));
    }
    strcpy(new_data, Data); 

    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);

    Data = new_data;
    Capacity = new_capacity; 
}

// Reserve memory, discarding the current of the buffer (if we expect to be fully rewritten)
void    Str::reserve_discard(int new_capacity)
{
    if (new_capacity <= Capacity)
        return;

    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);

    if (new_capacity < LocalBufSize)
    {
        // Disowned -> LocalBuf
        Data = local_buf();
        Capacity = LocalBufSize;
    }
    else
    {
        // Disowned or LocalBuf -> Heap
        Data = (char*)STR_MEMALLOC(new_capacity * sizeof(char));
        Capacity = new_capacity; 
    }
}

void    Str::shrink_to_fit()
{
    if (!Owned || is_using_local_buf())
        return;
    int new_capacity = length()+1;
    if (Capacity <= new_capacity)
        return;

    char* new_data = (char*)STR_MEMALLOC(new_capacity * sizeof(char)); 
    memcpy(new_data, Data, new_capacity);
    STR_MEMFREE(Data);
    Data = new_data;
    Capacity = new_capacity;
}

int     Str::setfv(const char* fmt, va_list args)
{
    // Needed for portability on platforms where va_list are passed by reference and modified by functions
    va_list args2;
    va_copy(args2, args);

    // MSVC returns -1 on overflow when writing, which forces us to do two passes
    // FIXME-OPT: Find a way around that.
#ifdef _MSC_VER
    int len = vsnprintf(NULL, 0, fmt, args);
    STR_ASSERT(len >= 0);

    if (Capacity < len+1)
        reserve_discard(len+1);
    len = vsnprintf(Data, len+1, fmt, args2);
#else
    // First try
    int len = vsnprintf(Owned ? Data : NULL, Owned ? Capacity : 0, fmt, args);
    STR_ASSERT(len >= 0);

    if (Capacity < len+1)
    {
        reserve_discard(len+1);
        len = vsnprintf(Data, len+1, fmt, args2);
    }
#endif

    Owned = 1;
    return len;
}

int     Str::setf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = setfv(fmt, args);
    va_end(args);
    return len;
}

int     Str::setfv_nogrow(const char* fmt, va_list args)
{
    int w = vsnprintf(Data, Capacity, fmt, args);
    Data[Capacity-1] = 0;
    Owned = 1;
    return (w == -1) ? Capacity-1 : w;
}

int     Str::setf_nogrow(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = setfv_nogrow(fmt, args);
    va_end(args);
    return len;
}
