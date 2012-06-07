//
//  record.cpp
//  DNS_proxy
//
//  Created by Henry Lee on 6/7/12.
//  Copyright (c) 2012 Henry Lee. All rights reserved.
//

#include "record.h"
#include <iostream>

namespace dns_proxy {
    
    Record::Record(const std::string &dbfolder)
    :record_(),
    ipv4_(NULL),
    db_(NULL),
    options_()
    {
        // Verify that the version of the library that we linked against is
        // compatible with the version of the headers we compiled against.
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        
        // Opening A Database
        // leveldb::DB* db;
        // leveldb::Options options;
        options_.create_if_missing = true;
        // Cache
        options_.block_cache = leveldb::NewLRUCache(kCacheSize);
        // NoCompression
        options_.compression = leveldb::kNoCompression;
        leveldb::Status status = leveldb::DB::Open(options_, dbfolder, &db_);
        if (!status.ok()) std::cerr << status.ToString() << std::endl;
        assert(status.ok());
    }
    /*
     *
     **/
    bool Record::Delete(const std::string& host)
    {
        std::cout << "not implemented" << std::endl;
        return false;
    }

    /*
     *
     **/
    std::string Record::GetIPv4Str(const std::string& host)
    {
        // record_.Clear();
        // get record
        std::string value, ipv4str;
        leveldb::Status s = db_->Get(leveldb::ReadOptions(), host, &value);
        if (s.ok()) {
            if (false == record_.ParseFromString(value)) {
                return std::string();
            }
            ipv4_ = record_.mutable_ipv4();
            int current = ipv4_->index();
            ipv4str = ipv4_->ipv4(current);
            ipv4_->set_index((++current) % ipv4_->ipv4_size());
            // write back to db
            record_.SerializeToString(&value);
            leveldb::WriteBatch batch;
            batch.Delete(host);
            batch.Put(host, value);
            s = db_->Write(leveldb::WriteOptions(), &batch);
            if (!s.ok()) {
                std::cerr << s.ToString() << std::endl;
            }
        } else {
            std::cerr << s.ToString() << host << std::endl;
            return std::string();
        }
        std::cout << record_.DebugString() << std::endl;
        std::cout << ipv4str << std::endl;
        return ipv4str;
    }
    /*
     *
     **/
    bool Record::PutIPv4(const std::string& host, const std::string& ipv4)
    {
        // record_.Clear();
        // get record
        std::string value;
        leveldb::Status s = db_->Get(leveldb::ReadOptions(), host, &value);
        //std::cout << value.length() << std::endl;
        if (s.ok()) {
            if (false == record_.ParseFromString(value)) {
                return false;
            }
            ipv4_ = record_.mutable_ipv4();
            for (int i = 0; i < ipv4_->ipv4_size(); ++i) {
                if (ipv4 == ipv4_->ipv4(i)) {
                    // already have the same ip
                    return false;
                }
            }
            // add to record
            ipv4_->add_ipv4(ipv4);
            record_.SerializeToString(&value);
            leveldb::WriteBatch batch;
            batch.Delete(host);
            batch.Put(host, value);
            s = db_->Write(leveldb::WriteOptions(), &batch);
            if (!s.ok()) {
                std::cerr << s.ToString() << std::endl;
                return false;
            }
        } else {
            record_.set_host(host);
            ipv4_ = record_.mutable_ipv4();
            ipv4_->add_ipv4(ipv4);
            ipv4_->set_index(0);
            
            record_.SerializeToString(&value);
            s = db_->Put(leveldb::WriteOptions(), host, value);
            if (!s.ok()) {
                std::cerr << s.ToString() << std::endl;
                return false;
            }
        }
        std::cout << record_.DebugString() << std::endl;
        return true;
    }
    
}