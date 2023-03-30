
#include "iostream"
#include "star/log.h"
#include "yaml-cpp/yaml.h"
#include "star/config.h"
auto port = star::Config::Lookup("system.port",8080,"bind port");

int main(){

    STAR_LOG_INFO(STAR_LOG_ROOT()) << port->getValue();

    star::Config::LoadFromFile("../../config/my.yaml");

    //STAR_LOG_ROOT()->setFormatter("%n -- %m --%n ");
    //std::cout << star::LogMgr::GetInstance()->toString()<<std::endl;
    STAR_LOG_ERROR(STAR_LOG_ROOT())<<"aaa";

    STAR_LOG_INFO(STAR_LOG_ROOT()) << star::Config::Lookup<int>("system.port")->getValue();

    STAR_LOG_INFO(STAR_LOG_ROOT()) << star::Config::Lookup<YAML::Node>("server1")->getValue()["ip"].as<std::string>();
    STAR_LOG_INFO(STAR_LOG_ROOT()) << star::Config::Lookup<YAML::Node>("server1")->getValue()["port"].as<std::string>();
    return 0;
}