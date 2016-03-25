#include <iostream>
#include <vector>
#include <thread>
#include <boost\asio.h>
#include "LockFree.h"

void generateTest(std::vector<std::vector<int>*> & data, int threads, int size)
{
    for (int i = 0; i < threads; ++i) {
        data[i] = new std::vector < int > ;
        data[i]->resize(size);
        for (int j = 0; j < data[i]->size(); ++j)
            (*data[i])[j] = rand() % size;
    }
}

void test()
{
    std::vector<std::vector<int>*> in;
    std::vector<std::vector<int>*> out;
    std::vector<std::thread*> t;
    int threadNumber = std::thread::hardware_concurrency();
    t.resize(threadNumber);
    generateTest(in, threadNumber, 1000);
    for (int i = 0; i < t.size(); ++i)
        t[i] = new thread(check, in[i], out[i]);
}

int main()
{
    stack<int> s;
    int a;
    int b;
    std::cin >> b;
    s.push(b);
    s.pop(a);
    std::cout << a << std::endl;
    system("pause");
    return 0;
}