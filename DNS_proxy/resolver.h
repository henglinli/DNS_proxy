//
//  resolver.h
//  DNS_proxy
//
//  Created by Henry Lee on 6/7/12.
//  Copyright (c) 2012 Henry Lee. All rights reserved.
//

#ifndef __DNS_proxy__resolver__
#define __DNS_proxy__resolver__

#include <stdint.h>
#include <arpa/inet.h>	//inet_addr , inet_ntoa , ntohs etc
#include <netinet/in.h>

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/utility.hpp>

#include "dns.h"

namespace dns_proxy {
    
    using boost::asio::ip::udp;
    class Resolver: private boost::noncopyable
    {
    public:
        Resolver(boost::asio::io_service& io_service,
                 const std::string &dns = kOpenDNS[0],
                 const uint16_t &port = kDNSPort)
        :socket_(io_service, udp::v4()),
        local_endpoint_(),
        remote_endpoint_(boost::asio::ip::address::from_string(dns), port),
        uuid_(boost::uuids::random_generator()()),
        header_(NULL),
        query_(),
        answer_(),
        qname_length_(0),
        data_(),
        answer_name_(),
        rdata_(NULL)
        {
            std::cout << "use dns: " << dns << std::endl;
        }
        /* TODO:
         * this big man should split
         * */
        std::size_t GetHostByName(const std::string &host,
                                  std::string *ipv4str = NULL,
                                  uint8_t* pdata = NULL,
                                  const int query_type = DNS_A);
    private:
        /*   www.google.com
         *  .www.google.com0
         *  3www6google3com0
         * */
        
        std::string ToDNSFormat(const std::string &user_format);
        /*   www.google.com
         *  .www.google.com0
         *  3www6google3com0
         * */
        
        bool ToDNSFormat(const std::string &user_format, uint8_t * dns_format);
        /*   www.google.com
         *  .www.google.com0
         *  3www6google3com0
         * */
        
        bool ToUserFormat(const uint8_t * dns_format, uint8_t * user_format)
        {
            return true;
        }
        /*   www.google.com
         *  .www.google.com0
         *  3www6google3com0
         * */
        
        std::string ToUserFormat(const uint8_t *dns_format)
        {
            std::string user_format("");
            for (int i=0; 0 != dns_format[i]; i++) {
                //statements
            }
            return user_format;
        }
        /*
         *
         * */
        uint8_t* ReadName(uint8_t* reader, uint8_t* buffer, int* count);
        udp::socket socket_;
        udp::endpoint local_endpoint_;
        udp::endpoint remote_endpoint_;
        boost::uuids::uuid uuid_;
        struct HEADER *header_;
        struct QUERY query_;
        struct RES_RECORD answer_;
        std::size_t qname_length_;
        uint8_t data_[512];
        uint8_t answer_name_[UINT8_MAX];
        uint8_t *rdata_;
    };
}

#endif /* defined(__DNS_proxy__resolver__) */
