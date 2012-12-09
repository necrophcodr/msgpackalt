#include <iostream>
#include <algorithm>
#include <iomanip>
#define MSGPACK_INLINE
#include "msgpackalt.hpp"
using namespace msgpackalt;
using namespace std;

void print_hex( byte c ) {
	cout << "0x" << hex << setw(2) << setfill('0') << ( int )c << " ";
}

int main( )
{
	packer packd;
	packd.pack_raw( "str\0\0str", 9 );
	cout << "*** Testing use of \\0 in std::string ***" << endl;
	string str = packd.string();
	cout << "Buffer len: " << packd.len() << endl;
	cout << "String len: " << str.size() << " (strlen=" << strlen(str.data()) << ")" << endl;
	cout << "Hex data:   ";
	for_each( str.begin(), str.end(), print_hex );
	cout << endl;
	cout << "C++ string: " << str << endl;
	cout << "C string:   " << str.data() << endl;
	cout << dec << endl;
	
	packer packed_msg;
	pack_dict msg;
	int request_id = 0;
	
	msg.pack("resource", "/foo")
       .pack("request_id", request_id)
       .pack("version", (uint32_t)1); 
	
	packed_msg << msg;
	str = packed_msg.string();
	cout << "Buffer len: " << packd.len() << endl;
	cout << "String len: " << str.size() << " (strlen=" << strlen(str.data()) << ")" << endl;
	cout << "Hex data:   ";
	for_each( str.begin(), str.end(), print_hex );
	cout << endl;
	cout << "C++ string: " << str << endl;
	cout << "C string:   " << str.data() << endl;
}
