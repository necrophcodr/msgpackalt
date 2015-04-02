msgpackalt is a **simple, fast and lite** binary serialisation library for C and C++. It implements the Message Pack protocol (http://msgpack.org/) in optimised pure standard-C for speed and portability with minimal overhead, and encoded streams are binary compatible with other implementations.

The Message Pack protocol itself works by interleaving the binary data you want to send with header bytes that specify its format, and therefore how to decode the stream. This produces an overhead of at most one byte per value for numeric types, and includes support for arrays, maps, strings and raw data.

Unlike other implementations, you need **only include a single file** in your project to use it! Includes out-of-the-box C++ support for both **STL** and **Qt4**.

See the [Why](Why.md) page for an explanation of the [rationale behind this implementation](Why.md), and why you should consider using it.

-- Martijn Jasperse, Jan 2013

### Demonstration ###
Here's a basic example of how to pack a few objects, **in C**
```
msgpack_p *p = msgpack_pack_init();
msgpack_pack_uint16( p, 12345u );
msgpack_pack_str( p, "pi" );
msgpack_pack_double( p, 3.14159 );
msgpack_pack_int64( p, -9876543210 );
```
and the same **in C++**
```
msgpackalt::packer p;
p << 12345u << "pi" << 3.14159 << -9876543210;
```
The resulting 24-byte buffer written in hex is ` CD3039A27069CB400921F9F01B866ED3FFFFFFFDB34FE916 `

Unpacking is then as simple as
```
char str[MAX_LEN]; uint32_t u32; double pi; int64_t i64;
u = msgpack_unpack_init( buffer, buffer_len );
msgpack_unpack_uint32( u, &u32 );
msgpack_unpack_str( u, str, MAX_LEN );
msgpack_unpack_double( u, &pi );
msgpack_unpack_int64( u, &i64 );
```
and again in C++:
```
std::string str; uint32_t u32; double pi; int64_t i64;
msgpackalt::unpacker u(p);
u >> u32 >> str >> pi >> i64;
```


### Dictionaries ###
A simplified C++ interface is provided to simplify packing _dictionaries_; string-indexed maps whose values may be of any type. The `package` object represents an item of packed data, so you can use your favourite associative container (e.g. std::map or QHash)
```
msgpackalt::packer p;
std::map<std::string, msgpackalt::package> dict, nested;
// create a simple nested dictionary
nested["num"] << 5;
nested["pi"] << 3.141f;
dict["test"] << "foo";
dict["bar"] << true;
dict["payload"] << nested /* nested dictionary */;
// serialise it
p << dict;
std::string str = p.string();
```
The contents of the resulting buffer `str` in hex is
` 83A474657374A3666F6FA3626172C3A77061796C6F616482A36E756D05A27069CA40490625 `

The elements can be unpacked with simple syntax, like
```
bool bar; float pi;
std::map<std::string, msgpackalt::package> d1 = str, d2;
d1["bar"] >> bar;
d1["payload"] >> d2; // get the nested dict
d2["pi"] >> pi;
cout << "Bar " << bar << ", pi " << pi << endl;
```

A more detailed example is available from the `examples` directory.