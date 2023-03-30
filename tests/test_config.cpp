
#include <list>
#include "star/log.h"
#include "star/config.h"
#include "yaml-cpp/yaml.h"
using namespace std;
auto port = star::Config::Lookup("system.port",8080,"bind port");
auto vec = star::Config::Lookup("system.vec",vector<int>{2,3},"vec");
//auto logss = star::Config::Lockup("logs",vector<string>(),"logs");
class Person{
public:
    Person() = default;
    Person(const string& name,int age): m_age(age), m_name(name){}
    int m_age = 0;
    std::string m_name ;
    std::string toString() const {
        std::stringstream ss;
        ss<<"[ name="<<m_name<<", age="<<m_age<<" ]";
        return ss.str();
    }
    bool operator==(const Person& oth){
        return m_age == oth.m_age && m_name == oth.m_name;
    }
};
namespace star{
template<>
class LaxicalCast<std::string ,Person>{
public:
    Person operator()(const std::string& str){
        YAML::Node node = YAML::Load(str);
        Person res;
        res.m_name = node["name"].as<std::string>();
        res.m_age = node["age"].as<int>();
        return res;
    }
};

template<>
class LaxicalCast<Person ,std::string >{
public:
    std::string operator()(const Person& v){
        YAML::Node node;
        node["name"] = v.m_name;
        node["age"] = v.m_age;
        std::stringstream ss;
        ss<<node;
        return ss.str();
    }
};
}
void test1(){
    STAR_LOG_DEBUG(STAR_LOG_ROOT())<<"Before \n"<<vec->toString();

    //YAML::Node config = YAML::LoadFile("config/log.yaml");
    star::Config::LoadFromFile("config/log.yaml");
    star::Config::Lookup("system.port","123"s,"vec");
    STAR_LOG_DEBUG(STAR_LOG_ROOT())<<"After \n"<<vec->toString();
    //cout<<star::LaxicalCast<string,vector<int>>()("- 1\n- 2\n- 5")[2];
    //cout<<star::LaxicalCast<list<list<vector<int>>>,string>()(list<list<vector<int>>>{{{1,2,4,5},{2,3}}});
    list<float> value{1.2,3.1415};
    cout<<star::LaxicalCast<map<string,list<float>>,string>()(map<string,list<float>> {{"star",value}});
}
void test2(){
    //auto pm = star::Config::Lookup("map",map<string,vector<Person>>(),"");

    //pm->setValue(map<string,vector<Person>>());
    auto a = star::Config::Lookup("class.person",Person(),"");
//    a->addListener(1,[](const auto& old_val, const auto& new_val){
//        STAR_LOG_INFO(STAR_LOG_ROOT())<<old_val.toString()<<" -> "<<new_val.toString();
//    });

    a->setValue(Person("aaa",44));
    //STAR_LOG_DEBUG(STAR_LOG_ROOT())<<"Before \n"<<a->toString()<<" "<<a->getValue().toString();
    //star::Config::LoadFromFile("config/log.yaml");
    STAR_LOG_ERROR(STAR_LOG_ROOT())<<"After \n"<<a->toString()<<" "<<a->getValue().toString();;
    //auto tmp = pm->getValue();
    star::Config::Visit([](star::ConfigVarBase::ptr config){
        STAR_LOG_WARN(STAR_LOG_ROOT())<<config->getName()<<" "<<config->getDescription()<<" "<<config->toString();
    });

    //auto b = a->getValue().toString();
}
int main(){
    test2();
    //std::cout<<star::LogMgr().GetInstance()->toYaml();
}
