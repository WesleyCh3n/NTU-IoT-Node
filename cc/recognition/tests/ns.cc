#include <iostream>
#include "nc.h"
#define LOG(x) std::cout << x << '\n'


int main(){
    LOG(cm::params::DETECT);
    LOG(cm::params::CLASSIFY);
    LOG(cm::params::RECOGNITION);
}
