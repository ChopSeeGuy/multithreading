#include <thread>
#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>
#include <numeric>
#include <iterator>
#include <chrono>
#include <assert.h>
#include <future>


constexpr short int minSize = 5000;

template <typename IT, typename T>
T t_accumulate (IT first, IT last, T init)
{
    const size_t distance = std::distance(first, last);
    const size_t maxThreads = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 2;
    if(distance > 0 and distance < minSize)
        return std::accumulate (first, last, init);
    else if (!distance)
        return init;
    
    const size_t neededThreads = std::min(distance/minSize, maxThreads);
    const size_t dataChunk = distance / neededThreads;

    std::vector<T> results (neededThreads);
    std::vector<std::future<T>> v_future (neededThreads);

    auto begin = first;

    for(size_t i = 0; i < neededThreads - 1; ++i)
    {
        auto end = std::next(begin, dataChunk);
        v_future[i] = std::async(std::launch::async, [](IT first, IT last)
            {
                return std::accumulate(first, last, T{});
            }, begin, end);
        begin = end;
    }

    results[neededThreads-1] = std::accumulate(begin, last, T{});

    for(size_t i = 0; i < neededThreads - 1; ++i)
    {
        results[i] = v_future[i].get();
    }

    return std::accumulate(results.begin(), results.end(), init);
}

void printTime (const decltype(std::chrono::high_resolution_clock::now()) &startStandard,
                const decltype(startStandard) &stopStandard,
                const decltype(startStandard) &startPararel,
                const decltype(startPararel) &stopPararel,
                const size_t &elements)
{
     std::cout << "\nAsync algorithm for [" << elements << "]: " 
		<< std::chrono::duration_cast<std::chrono::microseconds>(stopPararel - startPararel).count() 
		<< "us" << std::endl;
	std::cout << "\nNormal algorithm for [" << elements << "]: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(stopStandard - startStandard).count() 
		<< "us" << std::endl;
}

void useAlgorithm(const std::vector<int> &v)
{
    auto start = std::chrono::high_resolution_clock::now();
    size_t standardCount = std::accumulate(v.begin(), v.end(), 0);
    auto stop = std::chrono::high_resolution_clock::now();

    auto start3 = std::chrono::high_resolution_clock::now();
    size_t pararelCount = t_accumulate(std::begin(v), std::end(v), 0);
    auto stop3 = std::chrono::high_resolution_clock::now();

    assert(standardCount==pararelCount);

    printTime(start, stop, start3, stop3, v.size());
}

int main()
{
    std::vector <int> v (400'000'000);
    std::generate(v.begin(), v.end(),  [n=0] () mutable {return n++;});
    useAlgorithm(v);

    v.resize(100'000'000);
    useAlgorithm(v);

    v.resize(5'000);
    useAlgorithm(v);

    v.resize(2'500);
    useAlgorithm(v);
    
    return 0;
}