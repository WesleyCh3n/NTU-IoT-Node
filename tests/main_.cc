#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>

#include "boost/math/tools/norms.hpp"
using namespace std;
#define REP(i,n) for(int i=0; i<n; i++)

template<typename T>
auto readTSV(std::string fileName, std::vector< std::vector<T> > &vecs) -> void{
    std::ifstream inFile(fileName);
    std::string line;

    while(getline(inFile, line)){
        std::stringstream ss(line);
        std::vector<T> vec;
        std::string tmp;
        while(getline(ss, tmp, '\t')) {
            vec.push_back(std::stof(tmp));
        }
        vecs.push_back(vec);
    }
}

void* child(void* data) {
    char *str = (char*) data; // 取得輸入資料
    for(int i = 0;i < 10;++i) {
        printf("%s\n", str); // 每秒輸出文字
        sleep(1);
    }
    pthread_exit(NULL); // 離開子執行緒
}

int main(){
    std::vector< std::vector<float> > vecs;
    readTSV("./19-01.tsv", vecs);
    // for(auto &tmp: vecs[0]){
    //     cout << tmp << '\n';
    // }
    vector<float> dists;
    REP(i,4){
        REP(k,19){
            cout << boost::math::tools::l2_distance(vecs[i],vecs[k]) << '\n';
            // dists.push_back()
        }
        cout << '\n';
    }
    // float dist = boost::math::tools::l2_distance(vecs[0],vecs[1]);
    // cout << dist << '\n';

    // pthread_t t; // 宣告 pthread 變數
    // pthread_create(&t, NULL, child, (void*) "Child"); // 建立子執行緒
    //
    // // 主執行緒工作
    // for(int i = 0;i < 10;++i) {
    //     printf("Master\n"); // 每秒輸出文字
    //     sleep(1);
    // }
    //
    // pthread_join(t, NULL); // 等待子執行緒執行完成
    return 0;
}
