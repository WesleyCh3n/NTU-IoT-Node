#include "cxxopts.hpp"

int main(int argc, char** argv){
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
    options.add_options()
        ("i,image", "Run Image")
        ("s,stream", "Start streaming")
        ("h,help", "Print usage")
        ;
    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    if(result.count("stream")){
        std::cout << "strat streaming\n";
    }
    if(result.count("image")){
        std::cout << "Run image\n";
    }

    return 0;

}
