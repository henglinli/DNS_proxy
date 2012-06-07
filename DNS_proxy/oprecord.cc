#include <iostream>

#include "protobuf/protocol.pb.h"
#include "record.h"

int main(int argc, char* argv[]) {
#if 0
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    dns_proxy::dns_record record;
    record.set_host("www.google.com");
    dns_proxy::ipv4_t *ipv4 = record.mutable_ipv4();
    ipv4->add_ipv4("192.168.1.1");
    ipv4->add_ipv4("127.0.0.1");
    ipv4->add_ipv4("39.99.234.1");
    ipv4->set_index(0);
    
    std::string buffer;
    record.SerializeToString(&buffer);
    std::cout << record.DebugString() << std::endl;
    
    dns_proxy::dns_record another;
    another.ParseFromString(buffer);
    
    std::cout << another.host() << std::endl;
    ipv4 = another.mutable_ipv4();
    std::cout << ipv4->index() << std::endl;
    for (int i=0; i < ipv4->ipv4_size(); ++i) {
        std::cout << ipv4->ipv4(i) << std::endl;
    }
    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
    
#endif
   dns_proxy::Record newone;
#if 0
    newone.PutIPv4("www.google.com", "192.168.1.1");
    newone.PutIPv4("www.google.com", "192.168.1.2");
    newone.PutIPv4("www.google.com", "192.168.1.4");
    newone.GetIPv4Str("www.google.com");
    newone.PutIPv4("www.google.com", "192.168.1.9");
    newone.GetIPv4Str("www.google.com");
    newone.GetIPv4Str("www.google.com");
    newone.GetIPv4Str("www.google.com");
    newone.PutIPv4("www.google.com", "192.168.1.3");
#endif
    // get
    if (2 == argc) {
        newone.GetIPv4Str(argv[1]);
    } else if (3 == argc) {
        std::string isdelete("-d");
        if (argv[1] == isdelete) {
            //std::cout << "not implemented" << std::endl;
            newone.Delete(argv[2]);
        } else {
            newone.PutIPv4(argv[1], argv[2]);
        }
    } else {
        std::cout << "Usage-->" << std::endl;
        std::cout << "Delete:  " << argv[0] << " -d" << " g.cn" << std::endl;
        std::cout << "ADD: " << argv[0] << " g.cn" << " 192.168.1.1" << std::endl;
        std::cout << "Find: " << argv[0] << " g.cn" << std::endl;
    }
    return 0;
}
