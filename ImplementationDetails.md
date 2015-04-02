# Implementation Details #

Mostly _msgpackalt_ follows the conventions established by [Message Pack](http://msgpack.org/), but in several places it diverges in implementation, for reasons explained here.

### Packing and "no copy" ###
It really makes no sense to attempt to use a "zero copy" mechanism in this library, for the following reasons:
  1. At some point, the user wants to obtain a single string containing the concatenation of all the packed objects for storing/transmitting, at which point the pointers must be serialised anyway.
  1. The size of a pointer (4 or 8 bytes) is at least as large most values being written, plus requiring descriptive information.
  1. On little-endian systems, the endian-ness has to be changed, which _requires a copy_ to prevent data corruption.
  1. Automatic type conversion (e.g. storing a small U64 in a U16) means some objects will get type-cast, producing a new object (copy) _anyway_.
  1. There is no support for heterogeneous arrays (e.g. large floating point arrays) besides "raw data" which has no type information. Because the "array" object is heterogeneous it has to be interspersed with header bytes meaning a modified copy has to be made.
  1. It then makes sense to send large data arrays as "raw data" blocks preceded by a descriptive header, in which case it's probably desirable to send the raw data in separate packets.

Therefore this library uses a "minimum copy" implementation - data is copied exactly once, directly into the packed buffer, during which conversions are performed if desirable.


### Endian-ness ###
Because endian-conversions are so low-level, most compilers implement "built-ins" to perform them quickly. _msgpackalt_ opts to use these where possible (MSVC, GCC 4.3+) to gain speed improvements and not require linking to external libraries.

These are performed with the pre-processor macros `__LITTLE_ENDIAN__` and `__BIG_ENDIAN__`, of which exactly one must be defined. The GCC macro `__BYTE_ORDER__` is used to determine which the host system is, otherwise one can be defined manually at compile time. Currently mixed-endian systems are not supported, but could be by simply implementing relevant host-to-network order conversions.


### Unpack type conversion ###
In _Message Pack_, all unpacks in C are done via the maximum precision object, so a U8 unpacks via a U64, float via a double, etc. This is to maintain generality and enable a single "union" struct to be used in unpacking.

This is somewhat wasteful in terms of space and also puts the onus on the user to make sure they're unpacking what they expect (e.g. expect a U8, unpack a double, receive garbage). By implementing specific unpacking functions in the same way as packing functions, the interface is symmetric for ease of use, and performs automatic error checking - without the need for intermediate "object" copies.

Type conversions are performed by the compiler, which makes them efficient and endian-agnostic, as opposed to direct buffer manipulation techniques. This is the rationale behind implementing so many unpack functions in a seemingly copy-paste way: each function is (implicitly) performing a different conversion.

The possible down-side of this approach is that it is up to the user to perform the correct number of unpack operations following an `unpack_array()` or `unpack_map()` call. Given that the user has to iterate through `msgpack_object.via.array.ptr` to get the data anyway, this seems like no extra burden.