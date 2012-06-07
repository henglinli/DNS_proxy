//
//  main.cpp
//  DNS_proxy
//
//  Created by Henry Lee on 6/5/12.
//  Copyright (c) 2012 Henry Lee. All rights reserved.
//

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/dynamic_bitset.hpp>

#include "dns.h"
#include "record.h"
#include "resolver.h"

#define Debug (std::cout << __FUNCTION__ << std::endl)

namespace dns_proxy {
    using boost::asio::ip::udp;
    const uint16_t kDNSPort = 5553;
    const int kUDPMessgeBlockSize = 512;
    class DNSServer: private boost::noncopyable
    {
    public:
        /*
         *
         **/
        DNSServer(boost::asio::io_service& io_service):
        record_(),
        resolver_(io_service),
        recursion_desired_(false),
        valid_header_(false),
        socket_(io_service, udp::endpoint(udp::v4(), kDNSPort))
        {
            Debug;
            Receive();
        }
    private:
        /*
         *
         **/
        void Receive()
        {
            Debug;
            socket_.async_receive_from(boost::asio::buffer(data_, kUDPMessgeBlockSize), remote_endpoint_,
                                       boost::bind(&DNSServer::HandleReceive,
                                                   this,
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred));
        }
        /*
         *
         **/
        void HandleReceive(const boost::system::error_code& error,
                            std::size_t bytes_transferred)
        {
            Debug;
            std::cout << "async_receive_from: bytes_transferred --> " << bytes_transferred << std::endl;
            if (!error || error == boost::asio::error::message_size)
            {
                CheckHeader();
            }
        }
        /*
         *
         **/
        void CheckHeader()
        {
            Debug;
            header_ = (struct HEADER*)&data_;
            
            std::cout << "The query header:" << std::endl;
            // id 
            std::cout << "id(net) --> " << header_->id << std::endl;
            
            {
                uint16_t *ptmp = &(header_->id)+1;
                uint16_t tmp = *ptmp;
                boost::dynamic_bitset<> qr_rcode(16, tmp);
                std::cout << "ra--rcode --> " << qr_rcode << std::endl;
                std::cout << "qr --> " << qr_rcode[7] << std::endl;
                std::cout << "rd --> " << qr_rcode[0] << std::endl;
                // is query or only one query
                if (0 != qr_rcode[7] || 1 != htons(header_->qdcount)) {
                    std::cout << "Error ==> not query or not only one query" << std::endl;
                    valid_header_ = false;
                    // set rcode format error 1;
                    qr_rcode[8] = 0;
                    qr_rcode[9] = 0;
                    qr_rcode[10] = 0;
                    qr_rcode[11] = 1;
                    // set answers = 0;
                    header_->ancount = 0;
                    header_->nscount = 0;
                    header_->arcount = 0;
                    socket_.async_send_to(boost::asio::buffer(data_, 12), remote_endpoint_,
                                          boost::bind(&DNSServer::HandleAnother, this,
                                                      boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred));
                    
                    return;
                }
                recursion_desired_ = qr_rcode[15];
            }
            std::cout << ntohs(header_->qdcount) << " Questions." << std::endl;
            std::cout << ntohs(header_->ancount) << " Answers." << std::endl;
            std::cout << ntohs(header_->nscount) << " Authoritative Servers." << std::endl;
            std::cout << ntohs(header_->arcount) << " Additional records." << std::endl;
            CheckBody();
        }
        /*
         *
         **/
        void CheckBody()
        {
            reader_ = &data_[sizeof(struct HEADER)];
            query_.name = ReadName(reader_, data_, &stop_);
            std::string hostname((const char*)(query_.name));
            send_len = hostname.length();
            std::cout << "query name --> " << hostname << std::endl;
            
            // get ip
            // A form db
            std::string ipv4str = record_.GetIPv4Str(hostname);
            if (0 == ipv4str.length()) {
                send_len = resolver_.GetHostByName(hostname,
                                                   ipv4strs,
                                                   data_,
                                                   DNS_A);
                socket_.async_send_to(boost::asio::buffer(data_, send_len), remote_endpoint_,
                                      boost::bind(&DNSServer::HandleAnother, this,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred));
                //cache ip into db
                for (int i=0 ; i<20; ++i) {
                    if (0 == ipv4strs[i].length()) {
                        break;
                    }
                    record_.PutIPv4(hostname, ipv4strs[i]);
                }
                return;
            }
            std::cout << "get ip from dnsdb --> " << ipv4str << std::endl;
            //   www.google.com
            //  .www.google.com0
            //  3www6google3com0
            // move to answer section
            answer_.name = &data_[sizeof(struct HEADER)+send_len+2+sizeof(struct QUESTION)];
            // set name
            for (int i = 0; i < send_len+2; ++i) {
                answer_.name[i] = data_[sizeof(struct HEADER)+i];
            }
            // ToDNSFormat(hostname, answer_.name);
            // set answer counts
            header_->ancount = htons(1);
            // set type
            answer_.resource = (struct RES_DATA*)(answer_.name + send_len +2);
            answer_.resource->type = htons(DNS_A);
            answer_.resource->ttl = 0;
            answer_.resource->_class = htons(DNS_IN);
            answer_.resource->rdlength = htons(4);
            answer_.rdata = (uint8_t*)(answer_.resource) + sizeof(struct RES_DATA);
            uint32_t *ptmp = (uint32_t*) answer_.rdata;
            *ptmp = inet_addr(ipv4str.c_str());
            
            send_len = sizeof(struct HEADER) + send_len + 2 + sizeof(struct QUESTION)
                + send_len + 2 + sizeof(struct RES_DATA) + 4;
            socket_.async_send_to(boost::asio::buffer(data_, send_len), remote_endpoint_,
                                  boost::bind(&DNSServer::HandleAnother, this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred));
            
        }
        /*
         *
         **/
        void HandleAnother(const boost::system::error_code& /*error*/,
                         std::size_t /*bytes_transferred*/)
        {
            Debug;
            std::cout << "handle query done!" << std::endl;
            Receive();
        }
        /*
         *
         **/
        uint8_t* ReadName(uint8_t* reader, uint8_t* buffer, int* count)
        {
            uint8_t *name;
            name = answer_name_;
            unsigned int p=0,jumped=0,offset;
            int i , j;
            
            *count = 1;
            //name = (uint8_t*)malloc(256);
            
            name[0]='\0';
            
            //read the names in 3www6google3com format
            while(*reader!=0)
            {
                if(*reader>=192)
                {
                    offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
                    reader = buffer + offset - 1;
                    jumped = 1; //we have jumped to another location so counting wont go up!
                }
                else
                {
                    name[p++]=*reader;
                }
                
                reader = reader+1;
                
                if(jumped==0)
                {
                    *count = *count + 1; //if we havent jumped to another location then we can count up
                }
            }
            
            name[p]='\0'; //string complete
            if(jumped==1)
            {
                *count = *count + 1; //number of steps we actually moved forward in the packet
            }
            /*
             *
             * */
            //now convert 3www6google3com0 to www.google.com
            for(i=0;i<strlen((const char*)name);i++)
            {
                p=name[i];
                for(j=0;j<(int)p;j++)
                {
                    name[i]=name[i+1];
                    i=i+1;
                }
                name[i]='.';
            }
            name[i-1]='\0'; //remove the last dot
            return name;
        }
        /*   www.google.com
         *  .www.google.com0
         *  3www6google3com0
         * */
        bool ToDNSFormat(const std::string &user_format, uint8_t * dns_format)
        {
            std::string dns_format_ = "." + user_format + "p";
            std::size_t length = dns_format_.length();
            query_.name[length-1] = 0;
            for (int i=length-2,j=0; i>=0; --i) {
                if ('.' == dns_format_[i]) {
                    //std::cout << i << " " <<  j << std::endl;
                    dns_format[i] = j;
                    j = 0;
                } else {
                    ++j;
                    dns_format[i] = dns_format_[i];
                }
            }
            //query_.name[length] = '\0';
            return true;
        }
        /*
         *
         **/
        Record record_;
        Resolver resolver_;
        bool recursion_desired_;
        bool valid_header_;
        uint8_t data_[512];
        uint8_t answer_name_[UINT8_MAX];
        uint8_t *reader_;
        struct HEADER* header_;
        struct QUERY query_;
        struct RES_RECORD answer_;
        int stop_;
        udp::socket socket_;
        udp::endpoint remote_endpoint_;
        std::size_t send_len;
        std::string ipv4strs[20];
    };
}


int main(int argc, const char * argv[])
{
#if 0
    try
    {
        boost::asio::io_service io_service;
        dns_proxy::DNSServer server(io_service);
        
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
#else
    boost::asio::io_service io_service;
    dns_proxy::DNSServer server(io_service);
    
    io_service.run();
#endif
    return 0;
}

