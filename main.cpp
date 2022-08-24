#include "FastThreadPool.hpp"

#include <iostream>

int main() 
{
    auto task = [](int a, int b) -> decltype(auto) {
        int result = a + b;
        // std::this_thread::sleep_for(std::chrono::seconds(3));
        return result;
    };

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000000; ++i)
    {
        task(i, i + 1);
    }

    std::cout << "directed processing time : " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << std::endl;

    Jason::FastThreadPool pool(4);
    start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000000; ++i) 
    {
        // pool.Execute([i, &task](){ task(i, i + 1);});
        pool.Execute([&i, &task](){ task(i, i + 1);});
    }

    std::cout << "pool processing time1 : " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << std::endl;

    start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 10000000; ++i) 
    {
        // pool.Execute([i, &task](){ task(i, i + 1);});
        pool.Execute(task, i, i + 1);
    }

    std::cout << "pool processing time2 : " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << std::endl;

    return 0;

}