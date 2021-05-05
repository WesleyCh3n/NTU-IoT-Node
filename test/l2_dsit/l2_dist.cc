#include <iostream>
#include <array>
#include <vector>
#include "boost/math/tools/norms.hpp"

int main(){
    std::array<int, 3> a{1,1,1};
    std::array<int, 3> b{2,2,2};
    double dist1 = boost::math::tools::l2_distance(a, b);
    std::cout << dist1 << '\n';
    /*
     * Two Container need to be the same class
     * Below example won't work
     *  */
    // double dist2 = boost::math::tools::l2_distance(a, c);
    // std::cout << dist2 << '\n';
    // std::vector<int> c{2,2,2};
    return 0;
}
