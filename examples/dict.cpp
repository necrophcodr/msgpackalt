#include <iostream>
#include <algorithm>
#include <iomanip>
using namespace std;

#define MSGPACK_INLINE
#include "msgpackalt.hpp"
using namespace msgpackalt;

void print_hex( byte c ) { printf( "0x%02x ", c ); }

int main( )
{
	packer packd;
	pack_dict d1, d2;
	
	// create a simple dictionary
	d2.pack("foo",-3).pack("yyz",3.141f);
	// pack it into another dictionary
	d1.pack("test","foo").pack("bar",4).pack("msg",d2);
	
	// turn it into a string and print it
	std::string str = d1.string();
	cout << "*** Demonstrating nested dicts ***" << endl;
	for_each( str.begin(), str.end(), print_hex );
	cout << endl << endl;
	
	// unpack the nested dicts and print their key names
	unpack_dict u1( str ), u2 = u1["msg"];
	
	// print the keys to screen
	cout << "dict u1 contains entries ";
	for ( unpack_dict::const_iterator i = u1.begin(); i != u1.end(); ++i )
		cout << "'" << i->first << "' ";
	cout << endl << "dict u2 contains entries ";
	for ( unpack_dict::const_iterator i = u2.begin(); i != u2.end(); ++i )
		cout << "'" << i->first << "' ";
	cout << endl << endl;
	
	// extract individual elements
	int64_t foo; int64_t bar; double yyz; string test;
	u1.get("test",test);// returns false if fails
	cout << "test = " << test << endl;
	bar = u1["bar"];	// throws exception if fails
	cout << "bar  = " << bar << endl;
	u2.get("foo",foo);
	cout << "foo  = " << foo << endl;
	u2.get("yyz",yyz);
	cout << "yyz  = " << yyz << endl;
}
