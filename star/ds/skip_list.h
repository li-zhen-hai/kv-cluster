#ifndef STAR_SKIP_LIST_H
#define STAR_SKIP_LIST_H

#include <cstddef>
#include <cassert>
#include <ctime>
#include <utility>
#include <ostream>
#include "random.h"

namespace star{

template<typename K, typename V>
class SkipList;

template<typename K, typename V>
class Node {

    friend class SkipList<K, V>;

public:

    Node(int level) {
        forward = new Node<K,V>*[level+1];
        for(int i=0;i<=level;++i)
            forward[i] = nullptr;
    }

    Node(const K& k,const V& v,int level){
        key = k;
        value = v;
        forward = new Node<K,V>*[level+1];
        for(int i=0;i<=level;++i)
            forward[i] = nullptr;
    }

    ~Node(){
        delete[] forward;
    }

    K getKey() const { return key; }

    V getValue() const { return value; }

    void setValue(const V& val) { value = val;}

    //void setValue(V&& val) { value = val; }

    std::ostream& operator << (std::ostream& os) {
        os << "Key is "<<key<<",Value is " << value << std::endl;
        return os;
    }

protected:

    Node<K,V>* getLevel(int level){
        if(level > nodeLevel)
            return nullptr;
        return forward[level];
    }

    void setnodeLevel(int level) {
        nodeLevel = level;
        return ;
    }

private:
    K key;
    V value;
    Node<K, V> **forward;
    int nodeLevel;
};

template<typename K, typename V>
class SkipList {
public:

    using iterator = Node<K,V>*;

    enum CmpOperation{
        LessOperation,
        GreaterOperation,
    };


    SkipList(CmpOperation op = CmpOperation::LessOperation)
    :opera(op)
    ,rnd(0x12345678) {
        //createNode(MAX_LEVEL, header);
        header = new Node<K, V>(MAX_LEVEL);
        //需要初始化数组
        //注意:这里是level+1而不是level,因为数组是从0-level
        // node->forward = new Node<K, V> *[level + 1];
        //header->nodeLevel = MAX_LEVEL;
        header->setnodeLevel(MAX_LEVEL);
        nodeCount = 0;
    }

    ~SkipList() {
        freeList();
    }


    //注意:这里要声明成Node<K,V>而不是Node,否则编译器会把它当成普通的类
    iterator search(const K& key) const {
        Node<K, V> *node = header;
        for (int i = MAX_LEVEL; i >= 0; --i) {
            while (node->getLevel(i) && compare(node->getLevel(i)->key,key))
                node = node->getLevel(i);
        }
        if(!node || !node->forward[0])
            return nullptr;
        node = node->forward[0];
        if (node->key == key) {
            return node;
        } else {
            return nullptr;
        }
    }


    iterator insert(const K& key,const V& value) {
        Node<K, V> *update[MAX_LEVEL]={nullptr};

        Node<K, V> *node = header;

        for (int i = MAX_LEVEL; i >= 0; --i) {
            while (node->getLevel(i) && compare(node->getLevel(i)->key,key))
                node = node->getLevel(i);
            update[i] = node;
        }
        //首个结点插入时，node->forward[0]其实就是footer
        //node = node->forward[0];

        node = node->getLevel(0);

        //如果key已存在，则直接返回false
        if (node && node->key == key) {
            node->setValue(value);
            return node;
        }

        int nodeLevel = getRandomLevel();

        //创建新结点
        Node<K, V> *newNode;
        newNode = new Node<K, V>(key, value , MAX_LEVEL);
        //需要初始化数组
        // if (level > 0) {
        //     node->forward = new Node<K, V> *[level + 1];
        // }
        newNode->nodeLevel = nodeLevel;
        //createNode(nodeLevel, newNode, key, value);
        //cout << "----" << endl;
        //调整forward指针
        for (int i = nodeLevel; i >= 0; --i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        ++nodeCount;

        return newNode;
    }

    bool remove(const K& key) {
        V val;
        return remove(std::forward<K>(key),val);
    }

    bool remove(const K& key, V &value){
        Node<K, V> *update[MAX_LEVEL];
        Node<K, V> *node = header;
        for (int i = MAX_LEVEL; i >= 0; --i) {
            while (node->getLevel(i) && compare(node->getLevel(i)->key,key))
                node = node->getLevel(i);
            update[i] = node;
        }
        if(!node)
            return false;
        node = node->forward[0];
        //如果结点不存在就返回false
        if(!node)
            return false;
        if (node->key != key)
            return false;
        value = node->value;
        for (int i = 0; i <= MAX_LEVEL; ++i) {
            if (update[i]->forward[i] != node) {
                break;
            }
            update[i]->forward[i] = node->forward[i];
        }

        //释放结点
        delete node;

        --nodeCount;

        return true;
    }

    int size() {
        return nodeCount;
    }

    int getLevel() {
        return MAX_LEVEL;
    }

private:
    //释放表
    void freeList(){
        Node<K, V> *p = header;
        Node<K, V> *q;
        while (p != NULL) {
            q = p->forward[0];
            delete p;
            p = q;
        }
        delete p;
    }

    //随机生成一个level
    int getRandomLevel(){
        int level = static_cast<int>(rnd.Uniform(MAX_LEVEL));
        if (level == 0) {
            level = 1;
        }
        return level;
    }

    bool compare(const K& k1,const K& k2) const {
        if(opera == CmpOperation::LessOperation)
            return k1<k2;
        else
            return k1>k2;
    }

private:
    Node<K, V> *header;
    Node<K, V> *footer;

    CmpOperation opera;
   
    size_t nodeCount;

    static const int MAX_LEVEL = 16;

    Random rnd;
};

};

#endif 