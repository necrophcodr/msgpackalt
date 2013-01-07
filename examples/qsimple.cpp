/*
----------------------------------------------------------------------
MSGPACKALT :: a simple binary serialisation library
http://code.google.com/p/msgpackalt
----------------------------------------------------------------------
test.cpp : a demonstration of msgpackalt C++ classes
----------------------------------------------------------------------
*/
#define MSGPACK_INLINE
#define MSGPACK_QT
#include "msgpackalt.hpp"
using namespace msgpackalt;

#include <QDebug>

int main( )
{
	qDebug( "***** Simple example, QT version *****" );
	
	packer p;
	p << 12345u << QString( "pi" ).toAscii() << 3.14159 << -9876543210;
	
	QByteArray data = p.string( );
	qDebug() << data.toHex( );
	qDebug() << "Packed into" << data.size() << "bytes";
	qDebug() << "";
	
	QByteArray str;
	int64_t i64; double f; uint32_t n;
	
	unpacker u( data );
	try {
		u >> n >> str >> f >> i64;
		qDebug() << n << str << f << i64;
	} catch ( std::exception &e ) {
		qDebug( e.what());
	}
	return 0;
}
