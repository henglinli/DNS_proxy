//
//  record.h
//  DNS_proxy
//
//  Created by Henry Lee on 6/7/12.
//  Copyright (c) 2012 Henry Lee. All rights reserved.
//

#ifndef __DNS_proxy__record__
#define __DNS_proxy__record__

#include <arpa/inet.h> // for inet_addr
#include <cassert>
#include <string>
#include <boost/utility.hpp>

#include "leveldb/db.h"
#include "leveldb/cache.h"
#include "leveldb/write_batch.h"

#include "protobuf/protocol.pb.h"
namespace dns_proxy {
    const int kCacheSize = 10 * 1048576; // 10MB cache
    const char kDefaultDBFolder[] = "dnsdb";
    class Record: private boost::noncopyable {
    public:
        /*
         *
         **/
        Record(const std::string &dbfolder = kDefaultDBFolder);
        /*
         *
         **/
        bool Delete(const std::string& host);
        /*
         *
         **/
        std::string GetIPv4Str(const std::string& host);
        /*
         *
         **/
        inline uint32_t GetIPv4(const std::string& host)
        {
            std::string value = GetIPv4Str(host);
            //boost::asio::ip::address_v4 address_v4(value);
            //return address_v4.to_ulong();
            return inet_addr(value.c_str());
        }
        /*
         *
         **/
        bool PutIPv4(const std::string& host, const std::string& ipv4);
        /*
         *
         **/
        ~Record()
        {
            // Optional:  Delete all global objects allocated by libprotobuf.
            google::protobuf::ShutdownProtobufLibrary();
            //Closing A Database
            delete db_;
            delete options_.block_cache;
        }
    private:
        dns_proxy::dns_record record_;
        dns_proxy::ipv4_t *ipv4_;
        leveldb::DB* db_;
        leveldb::Options options_;
    };
}
#endif /* defined(__DNS_proxy__record__) */
