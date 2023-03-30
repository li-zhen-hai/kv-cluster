#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <leveldb/db.h>

#include "star/io_manager.h"
#include "star/log.h"
#include "star/time_measure.h"

void test_seq_write(){
    uint64_t begin = rand();
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());
    {
        star::TimeMeasure timer;
        std::map<std::string,std::string> kvs;
        for(int i=0;i<1000;i++){
            leveldb::Status s;
            do{
                db->Put(leveldb::WriteOptions(),std::to_string(begin+i),std::to_string(begin+i));
            }while(!s.ok());

        }
    }
    for(int i=0;i<1000;++i){
        std::string value;
        leveldb::Status s= db->Get(leveldb::ReadOptions(),std::to_string(begin+i),&value);
        if(value != std::to_string(begin+i))
            STAR_LOG_ERROR(STAR_LOG_ROOT()) << begin+i << " error!";
    }
    STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "test end!";
}

int main(){
    go test_seq_write;
    return 0;
}