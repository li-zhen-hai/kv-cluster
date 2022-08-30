#include "star/ds/skip_list.h"
#include "star/log.h"
#include "star/io_manager.h"
#include "star/time_measure.h"

#include <unordered_map>

static star::Logger::ptr  g_logger = STAR_LOG_NAME("test");

struct Key {
    int key;
    // ~Key() {
    //     STAR_LOG_DEBUG(g_logger) << "key ~" << key;
    // }

    bool operator > (const Key& k) const {
        return key < k.key;
    }

    bool operator < (const Key& k) const {
        return key > k.key;
    }

    bool operator == (const Key& k) const {
        return key == k.key;
    }

};

struct Value {
    int value;
    // ~Value() {
    //     STAR_LOG_DEBUG(g_logger) << "value ~" << value;
    // }
};

void test_add_node(star::SkipList<Key,Value>& slist) {
    for(int i=0;i<100000;++i) {
        usleep(100);
        slist.insert({i},{i});
    }
    return ;
}

void test_read_node(const star::SkipList<Key,Value>& slist) {
    for(int i=0;i<100000;++i) {
        usleep(1000);
        auto it = slist.search({i});
        if(it != nullptr)
            STAR_LOG_DEBUG(g_logger) << i << " find!";
        // else
        //     STAR_LOG_DEBUG(g_logger) << i << " not find!";
    }
    return ;
}

void test_time() {
    {
        STAR_LOG_DEBUG(g_logger) << "---hash begin---";
        std::unordered_map<long long,long long> stl_hash;
        star::TimeMeasure t;
        for(long long i=0;i<10000000;++i)
            stl_hash[i] = i;
        for(long long i=0;i<10000000;++i)
            if(stl_hash.find(i) == stl_hash.end()) {
                STAR_LOG_ERROR(g_logger) << "stl-hash can't find " << i;
            }
        STAR_LOG_DEBUG(g_logger) << "---hash end---";
    }
    {
        STAR_LOG_DEBUG(g_logger) << "---skiplist begin---";
        star::SkipList<long long,long long> skiplist;
        star::TimeMeasure t;
        for(long long i=0;i<10000000;++i){
            STAR_LOG_DEBUG(g_logger) << "skiplist insert " << i ;
            skiplist.insert(i,i);
        }
        for(long long i=0;i<10000000;++i)
            if(skiplist.search(i) == nullptr) {
                STAR_LOG_ERROR(g_logger) << "stl-hash can't find " << i;
            }
        STAR_LOG_DEBUG(g_logger) << "---skiplist end---";
    }
    return ;
}

int main() {
    //STAR_LOG_DEBUG(g_logger) << "-----";
    // star::SkipList<Key,Value>* slist = new star::SkipList<Key,Value>();
    // go [slist] {
    //     test_add_node(*slist);
    // };
    // go [slist] {
    //     test_read_node(*slist);
    // };
    test_time();
    return 0;
}