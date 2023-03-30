#include <cassert>
#include <iostream>
#include <string>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>


int main()
{
      leveldb::DB* db;
      leveldb::Options options;
      options.create_if_missing = true;
      leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
      assert(status.ok());

      std::string key = "test_key_";
      std::string value = "test_value_";
      std::string get;

      for(int i=0;i<100;++i){
            leveldb::Status s = db->Put(leveldb::WriteOptions(),key+std::to_string(i),value+std::to_string(i));
            if(!s.ok()){
                  std::cout<< "Write "<<i << " error!" << std::endl;
                  delete db;
                  return 0;
            }
      }

      leveldb::Iterator* iter = db->NewIterator(leveldb::ReadOptions());
      for(iter->SeekToFirst();iter->Valid();iter->Next()){
            std::cout << iter->key().ToString() <<" : " << iter->value().ToString() << std::endl;
      }

      // leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
      // // leveldb::WriteBatch batch;
      // // batch.Put(key,value);

      // if (s.ok())
      //       s = db->Get(leveldb::ReadOptions(), key, &get);
      // if (s.ok())
      //       std::cout << "key=" << key << "\nvalue=" << get  << std::endl;
      // else
      //       std::cout << "failed to find the key!" << std::endl;

      // const leveldb::Snapshot* shot = db->GetSnapshot();

      delete db;

      return 0;
}

