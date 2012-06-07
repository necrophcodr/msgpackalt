#include "speedtest.h"
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <stdio.h>

/* ********** DEFINE DUMMY HANDLERS ********** */
static int yajl_dummy_null( void * ctx )                                            { return 1; }
static int yajl_dummy_boolean( void * ctx, int boolean )                            { return 1; }
static int yajl_dummy_number( void * ctx, const char *s, unsigned int l )           { return 1; }
static int yajl_dummy_string( void * ctx, const unsigned char *s, unsigned int l )  { return 1; }
static int yajl_dummy_map_key( void * ctx, const unsigned char *s, unsigned int l ) { return 1; }
static int yajl_dummy_start_map( void * ctx )                                       { return 1; }
static int yajl_dummy_end_map( void * ctx )                                         { return 1; }
static int yajl_dummy_start_array( void * ctx )                                     { return 1; }
static int yajl_dummy_end_array( void * ctx )                                       { return 1; }

void test_yajl( test_t* t, int nobj )
{
    const unsigned char* raw;
	unsigned int raw_len;
    int i;
	
    /* ********** PACK MESSAGE ********** */
    {
        yajl_gen_config gcfg = {0, NULL};
	    yajl_gen g = yajl_gen_alloc(&gcfg);
        yajl_gen_array_open(g);
        for( i=0; i < nobj; ++i) {
            yajl_gen_array_open(g);
            yajl_gen_integer(g, t->id);
            yajl_gen_integer(g, t->width);
            yajl_gen_integer(g, t->height);
            yajl_gen_string(g, t->str, strlen(t->str));
            yajl_gen_array_close(g);
        }
        yajl_gen_array_close(g);
        yajl_gen_get_buf(g, &raw, &raw_len);
    }
    
    /* ********** UNPACK MESSAGE ********** */
    {
        yajl_parser_config hcfg = { 0, 0 };
        yajl_callbacks callbacks = {
            yajl_dummy_null,
            yajl_dummy_boolean,
            NULL,
            NULL,
            yajl_dummy_number,
            yajl_dummy_string,
            yajl_dummy_start_map,
            yajl_dummy_map_key,
            yajl_dummy_end_map,
            yajl_dummy_start_array,
            yajl_dummy_end_array
        };
        yajl_handle h = yajl_alloc(&callbacks, &hcfg, NULL);
        yajl_status stat = yajl_parse(h, raw, raw_len);
        if (stat != yajl_status_ok && stat != yajl_status_insufficient_data) {
            unsigned char * err = yajl_get_error(h, 1, raw, raw_len);
            fputs(err, stderr);
        }
        yajl_free(h);
    }
}

