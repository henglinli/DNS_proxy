//
//  resolver.cpp
//  DNS_proxy
//
//  Created by Henry Lee on 6/7/12.
//  Copyright (c) 2012 Henry Lee. All rights reserved.
//

#include "resolver.h"
#include <string.h>
#include <boost/dynamic_bitset.hpp>

namespace dns_proxy {
    using boost::asio::ip::udp;
    /*
     *
     * */
    std::size_t Resolver::GetHostByName(const std::string &host,
                                        std::string *ipv4str,
                                        uint8_t *pdata,
                                        const int query_type)
    {
        //A DNS packet typically looks like this HEADER-QUERY-RES_RECORD(n)
       
        if (NULL == pdata) {
            pdata = data_;
        }
        header_ = (struct HEADER *)pdata;
        //set header_HEADER
        {
            //header_->id
            pdata[0] = uuid_.data[0];
            pdata[1] = uuid_.data[1];
            {
                uint16_t *ptmp = (uint16_t *)&pdata;
                std::cout << "id=" << *ptmp << std::endl;
                std::cout << sizeof(HEADER) << std::endl;
            }
#if 0
            header_->qr = 0; //This is a query
            header_->opcode = 0; //This is a standard query
            header_->aa = 0; //Not Authoritative
            header_->tc = 0; //This message is not truncated
            header_->rd = 1; //Recursion Desired
            header_->ra = 0; //Recursion not available! hey we dont have it (lol)
            header_->z = 0;
            header_->ad = 0;
            header_->cd = 0;
            header_->rcode = 0;
            
            {
                uint16_t *ptmp = (uint16_t *)&pdata[2];
                uint16_t tmp = *ptmp;
                boost::dynamic_bitset<> bits_part(16, tmp);
                std::cout << tmp << std::endl;
                std::cout << bits_part << std::endl;
            }
#else
            boost::dynamic_bitset<> header_bits_part(16, 0UL);
            // rd
            header_bits_part[0] = 1;
            // tc
            header_bits_part[1] = 0;
            // aa
            header_bits_part[2] = 0;
            // opcode = [0,1,2] or[0000,0001,0010]
            header_bits_part[3] = 0;
            header_bits_part[4] = 0;
            header_bits_part[5] = 0;
            header_bits_part[6] = 0;
            // qr
            header_bits_part[7] = 0;
            
            // rcode
            header_bits_part[8] = 0;
            header_bits_part[9] = 0;
            header_bits_part[10] = 0;
            header_bits_part[11] = 0;
            // cd
            header_bits_part[12] = 0;
            // ad
            header_bits_part[13] = 0;
            // z
            header_bits_part[14] = 0;
            // ra
            header_bits_part[15] = 0;
            {
                uint16_t *ptmp = (uint16_t *)&pdata[2];
                uint16_t tmp = header_bits_part.to_ulong();
                *ptmp = tmp;
                std::cout << tmp << std::endl;
                std::cout << header_bits_part << std::endl;
            }
#endif
            header_->qdcount = htons(1); //we have only 1 question
            header_->ancount = 0;
            header_->nscount = 0;
            header_->arcount = 0;
        }
        //set query name
        query_.name = (uint8_t *)&pdata[sizeof(struct HEADER)];
        // important **********
        //  www.google.com
        // .www.google.com
        // 3www6google3com0
        size_t qname_length_ = host.length()+2;
        std::size_t data_len = sizeof(struct HEADER) + qname_length_ + sizeof(QUESTION);
        
        std::cout << "[" << host << "]" << std::endl;
#if 1
        ToDNSFormat(host, query_.name);
#else
        std::string dns_host = "." + host;
        std::size_t host_size = dns_host.length()+1;
        query_.name[host_size-1] = 'Q';//0;
        for (int i=host_size-2,j=0; i>=0; --i) {
            if ('.' == dns_host[i]) {
                query_.name[i] = 'Q';//j;
                //std::cout << j << std::endl;
                j = 0;
            } else {
                ++j;
                query_.name[i] = dns_host[i];
            }
        }
        
        query_.name[host_size+1] = '\0';
#endif
        for (int i = 0; i<qname_length_; ++i) {
            std::cout << query_.name[i];
        }
        std::cout << std::endl;
        
        query_.question = (struct QUESTION *)&pdata[sizeof(struct HEADER)+qname_length_]; //fill it
        query_.question->qtype = htons(query_type); //type of the query , A , MX , CNAME , NS etc
        query_.question->qclass = htons(1); //its internet (lol)
        //send query
        std::size_t pdatasize = sizeof(struct HEADER)+qname_length_+sizeof(struct QUESTION);
        socket_.connect(remote_endpoint_);
        std::size_t send_size = socket_.send(boost::asio::buffer(pdata, pdatasize));
        if (send_size != pdatasize) {
            std::cerr << "send_to error" << std::endl;
            std::abort();
        }
        std::cout << "send_size= " << send_size;
        std::size_t recv_size = socket_.receive(boost::asio::buffer(pdata, 512));
        std::cout << " recv_size= " << recv_size << std::endl;
        
        header_ = (struct HEADER *)pdata;
        
        std::cout << "The response header:" << std::endl;
        // id 
        std::cout << "id --> " << header_->id << std::endl;
        
        {
            uint16_t *ptmp = &(header_->id)+1;
            uint16_t tmp = *ptmp;
            boost::dynamic_bitset<> qr_rcode(16, tmp);
            std::cout << "ra--rcode --> " << qr_rcode << std::endl;
            std::cout << "qr --> " << qr_rcode[0] << std::endl;
            std::cout << "ra --> " << qr_rcode[8] << std::endl;
        }
        
        std::cout << ntohs(header_->qdcount) << " Questions." << std::endl;
        std::cout << ntohs(header_->ancount) << " Answers." << std::endl;
        std::cout << ntohs(header_->nscount) << " Authoritative Servers." << std::endl;
        std::cout << ntohs(header_->arcount) << " Additional records." << std::endl;
        //move to answer section
        uint8_t *reader = &pdata[sizeof(struct HEADER)+qname_length_+sizeof(struct QUESTION)];
        int stop = 0;
        
        std::cout << "== Answer Records ==" << std::endl;
        size_t rdlength = 0;
        //resource records in the answer section
        for (int i=0; i<ntohs(header_->ancount); ++i) {
            answer_.name = ReadName(reader, pdata, &stop);
            reader += stop;
            
            std::cout << "Question ==> " << i << std::endl <<" name: " << answer_.name << std::endl;
            
            answer_.resource = (struct RES_DATA*)reader;
            reader += sizeof(struct RES_DATA);
            std::cout << "Answer ==> " << i << std::endl;
            std::cout << " class: " << ntohs(answer_.resource->_class) << std::endl;
            std::cout << " ttl: " << ntohs(answer_.resource->ttl) << std::endl;
            rdlength = ntohs(answer_.resource->rdlength);
            std::cout << " rdlength: " << rdlength << std::endl;
            int ipv4str_counts = 0;
            std::string tmpstr;
            switch (ntohs(answer_.resource->type)) {
                case DNS_A:
                {
                    rdata_ = new uint8_t[ntohs(answer_.resource->rdlength)];
                    for (int j=0; j<ntohs(answer_.resource->rdlength); ++j) {
                        rdata_[j] = reader[j];
                    }
                    uint32_t *ipv4 = (uint32_t *)rdata_;
                    struct in_addr addr;
                    addr.s_addr = *ipv4;
                    tmpstr = (const char *)inet_ntoa(addr);
                    if (NULL != ipv4str) {
                        ipv4str[ipv4str_counts++] = tmpstr;
                    }
                    std::cout << " type: IPv4 --> "<< tmpstr << std::endl;
                    delete[] rdata_;
                    reader += ntohs(answer_.resource->rdlength);
                    break;
                }
                case DNS_CNAME:
                {
                    answer_.rdata = ReadName(reader, pdata, &stop);
                    reader += stop;
                    
                    std::cout << " type: Alias --> " << answer_.rdata << std::endl;
                    break;
                }
                default:
                {
                    answer_.rdata = ReadName(reader, pdata, &stop);
                    reader += stop;
                    
                    std::cout << "type: Others -->" << answer_.rdata << std::endl;
                }
            }
            
            data_len = data_len + sizeof(struct RES_RECORD) + strlen((const char*)(answer_.name)) + 2 + rdlength;
        }
        
        //name server resource records in the authority records section
        std::cout << "== Authoritive Records ==" << std::endl;
        for (int i=0; i<ntohs(header_->nscount); ++i) {
            answer_.name = ReadName(reader, pdata, &stop);
            reader += stop;
            std::cout << "Name: " << answer_.name << i << std::endl;
            
            answer_.resource = (struct RES_DATA *)reader;
            reader += sizeof(struct RES_DATA);
            
            answer_.rdata = ReadName(reader, pdata, &stop);
            reader += stop;
            
            if (DNS_NS == ntohs(answer_.resource->type)) {
                std::cout << " has name server ";
            } else {
                std::cout << " has others ";
            }
            data_len = data_len + sizeof(struct RES_RECORD) + strlen((const char*)(answer_.name)) + 2 + rdlength;
            std::cout << answer_.rdata << std::endl;
        }
        //resource records in the additional records section
        std::cout << "== Addttional Records ==" << std::endl;
        for (int i=0; i<ntohs(header_->arcount); ++i) {
            answer_.name = ReadName(reader, pdata, &stop);
            reader += stop;
            std::cout << "Name: " << answer_.name << i << std::endl;
            
            answer_.resource = (struct RES_DATA *)reader;
            reader += sizeof(struct RES_DATA);
            
            answer_.rdata = ReadName(reader, pdata, &stop);
            reader += stop;
            
            if (DNS_A == ntohs(answer_.resource->type)) {
                std::cout << " has IPv4 address ";
            } else {
                std::cout << " has others ";
            }
            data_len = data_len + sizeof(struct RES_RECORD) + strlen((const char*)(answer_.name)) + 2 + rdlength;
            std::cout << answer_.rdata << std::endl;
        }
        //std::cout << ToDNSFormat(host) << std::endl;
        return data_len;
    }
    
    /*   www.google.com
     *  .www.google.com0
     *  3www6google3com0
     * */
    std::string Resolver::ToDNSFormat(const std::string &user_format)
    {
        std::string dns_format = "." + user_format + "p";
        std::size_t length = dns_format.length();
        dns_format[length-1] = 0;
        for (int i=length-2,j=0; i>=0; --i) {
            if ('.' == dns_format[i]) {
                //std::cout << i << " " <<  j << std::endl;
                dns_format[i] = j;
                j = 0;
            } else {
                ++j;
            }
        }
        //dns_format[length] = '\0';
        return dns_format;
    }
    
    /*   www.google.com
     *  .www.google.com0
     *  3www6google3com0
     * */
    bool Resolver::ToDNSFormat(const std::string &user_format, uint8_t * dns_format)
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
     * */
    uint8_t* Resolver::ReadName(uint8_t* reader, uint8_t* buffer, int* count)
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
}
