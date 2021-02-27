#include <iostream>
#include <sstream>

int main(){
    std::stringstream buff;
    buff << "test" << '\n';
    buff << "test2" << '\n';
    std::cout << buff.str();
}
