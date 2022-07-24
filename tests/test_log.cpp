
#include "star/log.h"
#include "star/util.h"
#include <iostream>
#include <memory>
using namespace std;
int main(){
    //source::Logger::ptr logger(std::make_shared<source::Logger>());
    auto logger = star::LogMgr::GetInstance()->getLogger("system");
    STAR_LOG_ERROR(logger)<<"system";
  //cout<<"hello";
}
