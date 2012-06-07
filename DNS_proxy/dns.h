//
//  dns.h
//  DNS_proxy
//
//  Created by Henry Lee on 6/7/12.
//  Copyright (c) 2012 Henry Lee. All rights reserved.
//

#ifndef DNS_proxy_dns_h
#define DNS_proxy_dns_h

// open DNS
const char kOpenDNS[][15] = {"208.67.222.222", "208.67.220.220"};
// google DNS
const char kGoogleDNS[][15] = {"8.8.8.8", "8.8.4.4"};
// self DNS
const char kSelfDNS[][15]= {"127.0.0.1", "192.168.1.100"};
// DNS port
const uint16_t kDNSPort = 53;
const uint16_t kSelfDNSPort = 5553;

//Types of DNS resource records :)
enum dns_type {
    DNS_A = 1,        // Ipv4 address
    DNS_NS = 2,       // Nameserver
    DNS_CNAME = 5,    // canonical name
    DNS_SOA = 6,      // start of authority zone
    DNS_PTR = 12,     // domain name pointer
    DNS_MX = 15       // Mail server
};
enum dns_class {
    DNS_IN = 1,     //the Internet
    DNS_CS = 2,     //the CSNET class
                    //(Obsolete - used only for examples in some obsolete RFCs)
    DNS_CH = 3,     //the CHAOS class
    DNS_HS = 4,     //Hesiod [Dyer 87]
};
//DNS header structure
struct HEADER
{
	uint16_t id; // identification number
    
    //net endian
    uint8_t rd :1; // recursion desired
	uint8_t tc :1; // truncated message
	uint8_t aa :1; // authoritive answer
	uint8_t opcode :4; // purpose of message
	uint8_t qr :1; // query/response flag
    
	uint8_t rcode :4; // response code
	uint8_t cd :1; // checking disabled
	uint8_t ad :1; // authenticated data
	uint8_t z :1; // its z! reserved
	uint8_t ra :1; // recursion available
    // 
	uint16_t qdcount; // number of question entries
	uint16_t ancount; // number of answer entries
	uint16_t nscount; // number of authority entries
	uint16_t arcount; // number of resource entries
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct RES_DATA
{
	uint16_t type;
	uint16_t _class;
	uint32_t ttl;
	uint16_t rdlength;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
	uint8_t *name;
    struct RES_DATA *resource;
	uint8_t *rdata;
};

struct QUESTION {
    uint16_t qtype;
    uint16_t qclass;
};
//Structure of a Query
struct QUERY
{
	uint8_t *name;
	struct QUESTION *question;
};

#endif
