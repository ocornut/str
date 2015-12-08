// Str v0.1 
// Simple c++ string type with an optional local buffer
// https://github.com/ocornut/str

// LICENSE
// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy,
// distribute, and modify this file as you see fit.

// - This isn't a fully featured string class.
// - String are mutable. We don't maintain size so length() is not-constant time. Call reserve() to grow the buffer.
// - Maximum string size currently limited to 2 MB (we allocate 21 bits to hold capacity)
// - Local buffer size is currently limited to 1023 bytes (we allocate 10 bits to hold local buffer size)
// - This code hasn't been tested very much. it is probably incomplete or broken. I started making and using it for my own limited use.

// The idea is that you can provide an arbitrary sized local buffer if you expect string to fit most of the time,
// and then you avoid using costly heap.
//    Str16 s = "filename.h";
// But it can also use heap if necessary.
//    Str16 s = "long_filename_not_very_long_but_longer_than_expected.h";   // use heap
//    Str256 s = "long_filename_not_very_long_but_longer_than_expected.h";  // use local buffer
// Always use heap:
//    Str s = "hey";

// You can also copy references/literal pointer without allocation:
//    Str s;
//    s.set_ref("hey!");    // setter for literals/references
// Or via the helper constructor
//    StrRef("hey!");       // constructor for literals/reference

// You can cast any of StrXXX as a Str and it will still be functional. 
// Using a template e.g. Str<N> we could remove the LocalBufSize storage but it would make passing typed Str<> to functions tricky.
// Instead we don't use template so you can pass them around as the base type Str*. Also template are ugly.

// TODO:
// - More functions/helpers.

#pragma once

// Configuration
#ifndef STR_MEMALLOC
#define STR_MEMALLOC            malloc
#endif
#ifndef STR_MEMFREE
#define STR_MEMFREE             free
#endif
#ifndef STR_ASSERT
#define STR_ASSERT              assert
#include <assert.h>
#endif
#ifndef STR_SUPPORT_STD_STRING
#define STR_SUPPORT_STD_STRING  1
#endif

#ifdef STR_SUPPORT_STD_STRING
#include <string>
#endif

// This is the base class that you can pass around
// Footprint is 8-bytes (32-bits arch) or 16-bytes (64-bits arch)
class Str
{
    char*               Data;                   // Point to LocalBuf() or heap allocated
    int                 Capacity : 21;          // Max 2 MB
    int                 LocalBufSize : 10;      // Max 1023 bytes
    unsigned int        Owned : 1;              // 

public:
    inline const char*  c_str() const                           { return Data; }
    inline bool         empty() const                           { return Data[0] == 0; }
    inline int          length() const                          { return strlen(Data); }    // by design, but we could maintain it?
    inline int          capacity() const                        { return Capacity; }

    inline void         set_ref(const char* src);
    int                 setf(const char* fmt, ...);
    int                 setfv(const char* fmt, va_list args);
    int                 setf_nogrow(const char* fmt, ...);
    int                 setfv_nogrow(const char* fmt, va_list args);

    void                clear();
    void                reserve(int cap);
    void                reserve_discard(int cap);
    void                shrink_to_fit();

    inline char&        operator[](size_t i)                    { return Data[i]; }
    inline char         operator[](size_t i) const              { return Data[i]; }
    //explicit operator const char*() const{ return Data; }

    inline Str();
    inline Str(const char* rhs);
    inline void         set(const char* src);
    inline Str&         operator=(const char* rhs)              { set(rhs); return *this; }
    inline bool         operator==(const char* rhs) const       { return strcmp(c_str(), rhs) == 0; }

    inline Str(const Str& rhs);
    inline void         set(const Str& src);
    inline Str&         operator=(const Str& rhs)               { set(rhs); return *this; }
    inline bool         operator==(const Str& rhs) const        { return strcmp(c_str(), rhs.c_str()) == 0; }

#if STR_SUPPORT_STD_STRING
    inline Str(const std::string& rhs);
    inline void         set(const std::string& src);
    inline Str&         operator=(const std::string& rhs)       { set(rhs); return *this; }
    inline bool         operator==(const std::string& rhs)const { return strcmp(c_str(), rhs.c_str()) == 0; }
#endif

protected:
    inline char*        local_buf()                             { return (char*)this + sizeof(Str); }
    inline const char*  local_buf() const                       { return (char*)this + sizeof(Str); }
    inline bool         is_using_local_buf() const              { return Data == local_buf() && LocalBufSize != 0; }

    // Constructor for StrXXX variants with local buffer
    Str(unsigned short local_buf_size)
    {
        STR_ASSERT(local_buf_size < 1024);
        Data = local_buf();
        Data[0] = '\0';
        Capacity = local_buf_size;
        LocalBufSize = local_buf_size;
        Owned = 1;
    }
};

void    Str::set(const char* src)
{
    int buf_len = strlen(src)+1;
    if ((int)Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src, buf_len);
    Owned = 1;
}

void    Str::set(const Str& src)
{
    int buf_len = strlen(src.c_str())+1;
    if ((int)Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src.c_str(), buf_len);
    Owned = 1;
}

#if STR_SUPPORT_STD_STRING
void    Str::set(const std::string& src)
{
    int buf_len = (int)src.length()+1;
    if ((int)Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src.c_str(), buf_len);
    Owned = 1;
}
#endif

inline void Str::set_ref(const char* src)
{
    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);
    Data = (char*)src;
    Capacity = 0;
    Owned = 0;
}

Str::Str()
{
    Data = "";      // Shared read-only initial buffer for 0 capacity
    Capacity = 0;
    LocalBufSize = 0;
    Owned = 0;
}

Str::Str(const Str& rhs)
{
    Data = "";
    Capacity = 0;
    LocalBufSize = 0;
    Owned = 0;
    set(rhs);
}

Str::Str(const char* rhs)
{
    Data = "";
    Capacity = 0;
    LocalBufSize = 0;
    Owned = 0;
    set(rhs);
}

#if STR_SUPPORT_STD_STRING
Str::Str(const std::string& rhs)
{
    Data = "";
    Capacity = 0;
    LocalBufSize = 0;
    Owned = 0;
    set(rhs);
}
#endif

// Literal/reference string
class StrRef : public Str
{
public:
    StrRef(const char* s) : Str() { set_ref(s); }
};

// Types embedding a local buffer
// NB: we need to override the constructor and = operator for both Str& and TYPENAME (without the later compiler will call a default copy operator)
#if STR_SUPPORT_STD_STRING

#define STR_DEFINETYPE(TYPENAME, LOCALBUFSIZE)                                      \
class TYPENAME : public Str                                                         \
{                                                                                   \
    char local_buf[LOCALBUFSIZE];                                                   \
public:                                                                             \
    TYPENAME() : Str(LOCALBUFSIZE) {}                                               \
    TYPENAME(const Str& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                      \
    TYPENAME(const TYPENAME& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                 \
    TYPENAME&   operator=(const char* rhs)          { set(rhs); return *this; }     \
    TYPENAME&   operator=(const Str& rhs)           { set(rhs); return *this; }     \
    TYPENAME&   operator=(const TYPENAME& rhs)      { set(rhs); return *this; }     \
    TYPENAME&   operator=(const std::string& rhs)   { set(rhs); return *this; }     \
};                                                                                  

#else

#define STR_DEFINETYPE(TYPENAME, LOCALBUFSIZE)                                      \
class TYPENAME : public Str                                                         \
{                                                                                   \
    char local_buf[LOCALBUFSIZE];                                                   \
public:                                                                             \
    TYPENAME() : Str(LOCALBUFSIZE) {}                                               \
    TYPENAME(const Str& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                      \
    TYPENAME(const TYPENAME& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                 \
    TYPENAME&   operator=(const char* rhs)          { set(rhs); return *this; }     \
    TYPENAME&   operator=(const Str& rhs)           { set(rhs); return *this; }     \
    TYPENAME&   operator=(const TYPENAME& rhs)      { set(rhs); return *this; }     \
};

#endif

// Declaring a few types here
STR_DEFINETYPE(Str16, 16)
STR_DEFINETYPE(Str32, 32)
STR_DEFINETYPE(Str64, 64)
STR_DEFINETYPE(Str128, 128)
STR_DEFINETYPE(Str256, 256)

//-------------------------------------------------------------------------
