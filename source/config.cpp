
#include <algorithm>
#include <cctype>
#include "star/config.h"

namespace star {
static Logger::ptr g_logger = STAR_LOG_NAME("system");

static void ListAllMenbers(const std::string& perfix,const YAML::Node& node,
                           std::list<std::pair<std::string,YAML::Node>>& output){
    if(perfix.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789._")
       != std::string::npos){
        STAR_LOG_ERROR(STAR_LOG_ROOT())<<"config invalid name : "<<perfix<<" : "<<node;
        return;
    }
    output.emplace_back(perfix,node);
    if(node.IsMap()){
        for(auto& item : node){
            std::string name = perfix.empty()? item.first.Scalar() : perfix + "." + item.first.Scalar();
            ListAllMenbers(name,item.second,output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node &root) {
    //RWMutex::WriteLock lock(GetDatas());
    std::list<std::pair<std::string,YAML::Node>> allNodes;
    ListAllMenbers("",root,allNodes);
    for(auto& item:allNodes){
        std::string key = item.first;
        if(key.empty())
            continue;

        //STAR_LOG_DEBUG(STAR_LOG_ROOT())<<key<<" = "<<item.second<<std::endl;

        std::transform(key.begin(),key.end(),key.begin(),::tolower);
        auto var = LookupBase(key);
        if(var){
            if(item.second.IsScalar()){
                var->fromString(item.second.Scalar());
            } else{
                std::stringstream ss;
                ss << item.second;
                var->fromString(ss.str());
            }
            //STAR_LOG_INFO(STAR_LOG_ROOT()) << var->toString();
        }else{
            auto v = std::make_shared<ConfigVar<YAML::Node>>(key,item.second,key);
            GetDatas()[key] = v;
        }
    }
}

void Config::LoadFromFile(const std::string &file) {
    //STAR_LOG_INFO(STAR_LOG_ROOT())<<"load file:"<<file;
    YAML::Node root = YAML::LoadFile(file);
    LoadFromYaml(root);
}

ConfigVarBase::ptr Config::LookupBase(const std::string &name) {
    RWMutex::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if(it == GetDatas().end())
        return nullptr;
    return it->second;
}

void Config::Visit(std::function<void(const ConfigVarBase::ptr)> cb) {
    auto& config = GetDatas();
    for(auto& item: config){
        cb(item.second);
    }
}

}