@set MSGPACK=../../msgpack-0.5.4
@set MSGPACKALT=../
@set PROTOBUF=../../protobuf-2.4.1
@set YAJL=../../yajl-0.4.0/build/yajl-0.4.0
cl /Ox /EHsc /MD /Fespeedtest /I%MSGPACK%/src /I%PROTOBUF%/src /I%YAJL%/include /I%MSGPACKALT% speedtest.c test_msgpackalt.c test_yajl.c /Tp test_protobuf.cpp test_msgpack.cpp /link %MSGPACK%/lib/msgpack.lib %PROTOBUF%/vsprojects/Release/libprotobuf.lib %YAJL%/lib/Release/yajl_s.lib ws2_32.lib
@del *.bak *.obj
