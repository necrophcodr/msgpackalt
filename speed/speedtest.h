#ifndef SPEEDTEST_H
#define SPEEDTEST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int id, width, height;
    const char* str;
} test_t;

void test_msgpackalt( test_t* t, int nobj );
void test_msgpack( test_t* t, int nobj );
void test_protobuf( test_t* t, int nobj );
void test_yajl( test_t* t, int nobj );

#ifdef __cplusplus
}
#endif

#endif
