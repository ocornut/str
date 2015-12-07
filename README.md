Simple c++ string type with an optional local buffer

```
Str v0.1 
Simple c++ string type with an optional local buffer
https://github.com/ocornut/str

LICENSE
This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy,
distribute, and modify this file as you see fit.

- This isn't a fully featured string class.
- String are mutable. We don't maintain size so length() is not-constant time. 
- Maximum string size currently limited to 2 MB (we allocate 21 bits to hold capacity)
- Local buffer size is currently limited to 1023 bytes (we allocate 10 bits to hold local buffer size)
- In "non-owned" mode for literals/reference we don't do any tracking/counting of references.
- This code hasn't been tested very much. it is probably incomplete or broken. Made it for my own use.

The idea is that you can provide an arbitrary sized local buffer if you expect string to fit 
most of the time, and then you avoid using costly heap.

No local buffer, always use heap, sizeof()==8

   Str s = "hey";

Local buffer of 16 bytes, sizeof() == 8+16 bytes.

   Str16 s = "filename.h"; // copy into local buffer
   Str16 s = "long_filename_not_very_long_but_longer_than_expected.h";   // use heap

Local buffer of 256 bytes, sizeof() == 8+256 bytes.

   Str256 s = "long_filename_not_very_long_but_longer_than_expected.h";  // copy into local bufer

Format string helpers:

   Str256 filename;
   filename.setf("%s/%s.tmp", folder, filename);

You can pass your StrXXX* as a Str* to functions and it will still be functional. 

You can also copy references/literal pointer without allocation:

   Str s;
   s.set_ref("hey!");	// setter for literals/references
   
Or via the helper constructor

   StrRef("hey!");		// constructor for literals/reference

(Using a template e.g. Str<N> we could remove the LocalBufSize storage but it would make passing 
typed Str<> to functions tricky. Instead we don't use template so you can pass them around as
the base type Str*. Also, templates are ugly.)
```
