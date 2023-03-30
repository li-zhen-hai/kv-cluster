
#include "star/star.h"
#include <vector>
#include <iostream>
void test( int n){
    //constexpr int c=0;
    STAR_ASSERT2(0,"Not zero");
    //STAR_STATIC_ASSERT(c);
    //star::Assert(0,"Void test(int)");
}
int main(){
    std::vector<std::string> vec{"aa","bb"};

    std::cout<<vec.size();
}