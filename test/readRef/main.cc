#include <iostream>
#include <fstream>

int main(){
    std::ifstream inFile("./ref_dict.csv");
    std::string id;
    while(getline(inFile, id)){
        std::cout << std::stoi(id) << '\n';
    }
    return 0;
}
