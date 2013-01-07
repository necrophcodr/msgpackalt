#define MSGPACK_INLINE
#define MSGPACK_QT
#include "msgpackalt.hpp"
using namespace msgpackalt;

#include <QHash>
typedef QHash<QByteArray,package> pack_dict;
#include <QDebug>

int main( )
{
	packer packd;
	pack_dict d1, d2;
	
	// create a simple dictionary
	d2["foo"] << -65537;
	d2["yyz"] << 3.141f;
	// pack it into another dictionary
	d1["test"] << "foobar";
	d1["bar"] << -(1ll<<33);
	d1["msg"] << d2;
	qDebug( "*** Demonstrating QT nested dicts ***" );
	packd << d1;
	
	// turn it into a string and print it
	QByteArray str = packd.string( );
	qDebug() << str.toHex( );
	qDebug() << "";
	
	// unpack the nested dicts and print their key names
	unpacker unpackd( str );
	pack_dict u1, u2;
	unpackd >> u1;
	u1["msg"] >> u2;
	
	// print the keys to screen
	qDebug( "Dict u1 contains entries:" );
	for ( pack_dict::const_iterator i = u1.begin(); i != u1.end(); ++i )
		qDebug() << " -" << i.key( );
	qDebug( "Dict u2 contains entries:" );
	for ( pack_dict::const_iterator i = u2.begin(); i != u2.end(); ++i )
		qDebug() << " -" << i.key( );
	qDebug() << "";
	
	// extract individual elements
	long long foo; int64_t bar; double yyz; QByteArray test;
	u1["test"] >> test;
	qDebug() << "test =" << test;
	u1["bar"] >> bar;
	qDebug() << "bar  =" << bar;
	u2["foo"] >> foo;
	qDebug() << "foo  =" << foo;
	u2["yyz"] >> yyz;
	qDebug() << "yyz  =" << yyz;
	return 0;
}
