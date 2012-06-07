#include "speedtest.h"
#include <msgpack.hpp>

void test_msgpack( test_t* t, int nobj )
{
    msgpack::sbuffer raw;
	{
		msgpack::packer<msgpack::sbuffer> pk(raw);
		for(unsigned int i=0; i < nobj; ++i) {
			pk << t->id << t->width << t->height << std::string( t->str );
		}
	}
    
    {
		msgpack::zone z;
		size_t off = 0;
		for(unsigned int i=0; i < nobj; ++i) {
			msgpack::object obj = msgpack::unpack(raw.data(), raw.size(), z, &off);
			obj.convert();
		}
	}
}

