#include <iostream>
#include <algorithm>
#include <iomanip>
using namespace std;

#define MSGPACK_INLINE
#include "msgpackalt.hpp"
using namespace msgpackalt;

void print_hex( byte c ) {
	cout << "0x" << hex << setw(2) << setfill('0') << ( int )c << " ";
}

int main( )
{
	packer packd;
	pack_dict d1, d2;
	// pack a nested dictionary example
	d2.pack("foo",1).pack("yyz",3.141);
	d1.pack("test","foo").pack("bar",-2).pack("msg",d2);
	std::string str = d1.string();
	
	// print to screen
	cout << "*** Demonstrating nested dicts ***" << endl;
	for_each( str.begin(), str.end(), print_hex );
	cout << endl << endl;
	
	// unpack the nested dicts and print their key names
	unpack_dict u1( str ), u2;
	cout << "dict u1 contains entries ";
	for ( unpack_dict::const_iterator i = u1.begin(); i != u1.end(); ++i )
		cout << "'" << i->first << "' ";
	u1.get("msg",u2);
	cout << endl << "dict u2 contains entries ";
	for ( unpack_dict::const_iterator i = u2.begin(); i != u2.end(); ++i )
		cout << "'" << i->first << "' ";
	cout << endl;
	
	// extract individual elements
	int foo; unsigned int bar; double yyz; string test;
	bar = u1["bar"];	// throws exception if fails
	u1.get("test",test);// returns false if fails
	foo = u2["foo"];
	u2.get("yyz",yyz);
	cout << endl << "{bar=" << bar << ", test=" << test << ", msg={foo=" << foo << ", yyz="<< yyz << "}}" << endl;
	
}
