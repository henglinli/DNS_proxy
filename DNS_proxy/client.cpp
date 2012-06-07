//
//  client.cpp
//  proxy
//
//  Created by Henry Lee on 6/5/12.
//  Copyright (c) 2012 Henry Lee. All rights reserved.
//
//  copy form
//  http://www.binarytides.com/blog/dns-query-code-in-c-with-linux-sockets/
//  http://www.binarytides.com/blog/dns-query-code-in-c-with-winsock/
//
//DNS Query Program on Linux
//Author : Silver Moon (m00n.silv3r@gmail.com)
//Dated : 29/4/2009
#include <stdint.h>
#include <arpa/inet.h>	//inet_addr , inet_ntoa , ntohs etc
#include <netinet/in.h>

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/dynamic_bitset.hpp>

#include "resolver.h"

int main(int argc, const char * argv[])
{
    boost::asio::io_service io_service;
#if 1
    dns_proxy::Resolver resolver(io_service, kSelfDNS[0], kSelfDNSPort);//kGoogleDNS[0]);//kOpenDNS[0]);
    if (argc>1) {
        resolver.GetHostByName(argv[1]);
    } else {
        resolver.GetHostByName("www.163.com");
    }
#else
    dns_proxy::Resolver resolver(io_service);
    if (argc>1) {
        resolver.GetHostByName(argv[1]);
    } else {
        resolver.GetHostByName("www.163.com");
    }

    //dns_proxy::Client client(io_service);
#endif
    return 0;
}