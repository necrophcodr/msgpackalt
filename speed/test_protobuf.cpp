#include "speedtest.h"

#include <limits>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "test_proto.pb.h"

void test_protobuf( test_t* t, int nobj )
{
	proto_test::Test target;
	std::string raw;
    
    target.set_id(t->id);
	target.set_width(t->width);
	target.set_height(t->height);
	target.set_data( std::string(t->str) );
    
	{
		google::protobuf::io::StringOutputStream output(&raw);
		google::protobuf::io::CodedOutputStream encoder(&output);
		for(unsigned int i=0; i < nobj; ++i) {
			encoder.WriteVarint32(target.ByteSize());
			target.SerializeToCodedStream(&encoder);
		}
	}

	{
		proto_test::Test msg;
		google::protobuf::io::ArrayInputStream input(raw.data(), raw.size());
		google::protobuf::io::CodedInputStream decoder(&input);
		decoder.SetTotalBytesLimit(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
		for(unsigned int i=0; i < nobj; ++i) {
			unsigned int limit = 0; int old;
			decoder.ReadVarint32(&limit);
			old = decoder.PushLimit(limit);
			msg.ParseFromCodedStream(&decoder);
			decoder.PopLimit(old);
		}
	}
}

#include "test_proto.pb.cc"
