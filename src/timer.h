#ifndef __TIMER_H__
#define __TIMER_H__
#include <iostream>

class Timer{
    public:
        Timer(){
            stTimePt_ = std::chrono::high_resolution_clock::now();
        }
        ~Timer(){
            Stop();
        }
        void Stop(){
            auto endTimePt = std::chrono::high_resolution_clock::now();
            auto start = std::chrono::time_point_cast<std::chrono::microseconds>
                         (stTimePt_).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>
                         (endTimePt).time_since_epoch().count();
            auto duration = end - start;
            std::cout << "take " << duration << "us ("
                      << duration*0.001 << "ms)\n";
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> stTimePt_;
};

#endif
