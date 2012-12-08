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
	cout << "Internal len:   " << packd.len() << endl;
	cout << "String len:     " << str.size() << endl;
	cout << "Representation: ";
	for_each( str.begin(), str.end(), print_hex );
	cout << endl;
	cout << "Direct print:   " << str << endl;
	cout << "Via c-string:   " << str.data() << endl;
	cout << dec << endl;
	
	packer packed_msg;
	pack_dict msg;
	int request_id = 0;
	
	msg.pack("method", "get")
       .pack("type", "request")
       .pack("resource", "/foo")
       .pack("request_id", request_id)
       .pack("version", (uint32_t)1); 
	
	packed_msg << msg;
	str = packed_msg.string();
	cout << "Internal len:   " << packed_msg.len() << endl;
	cout << "String len:     " << str.size() << endl;
	cout << "Representation: ";
	for_each( str.begin(), str.end(), print_hex );
	cout << endl;
	cout << "Direct print:   " << str << endl;
	cout << "Via c-string:   " << str.data() << endl;
}
