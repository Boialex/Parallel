#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <assert.h>
#include <fstream>

// судя по системным данным размер кеш-линии 64 байта, поэтому создадим массив чаров(1байт) размера 
// 65 байт, чтобы первый и последний элементы были точно в разных кеш-линиях
char a[65];
static const int iterations = 1E8;

// создадим функцию посложнее чем ++a, что процессор ее не оптимизировал слишком сильно и была заметна разница
int somefunc(const int a)
{
    return (a + 2) % 100 * 2 + a / 2 * 1;
}

void f(const int pos)
{
    for (int i = 0; i < iterations; ++i)
        a[pos] = somefunc(a[pos]);
}

int main()
{
    std::vector<std::thread*> t(2);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    // with PingPong - 2 threads working with 1 cache line
    t[0] = new std::thread(f, 0);
    t[1] = new std::thread(f, 1);
    for (int i = 0; i < 2; ++i) {
        t[i]->join();
    }
    end = std::chrono::system_clock::now();
    long long pingpong = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    start = std::chrono::system_clock::now();
    // without PingPong - 2 threads working with 2 cache lines
    t[0] = new std::thread(f, 0);
    t[1] = new std::thread(f, 64);
    for (int i = 0; i < 2; ++i) {
        t[i]->join();
    }
    end = std::chrono::system_clock::now();
    long long withoutpingpong = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::fstream cout;
    cout.open("output.txt");
    //cout.clear();
    cout << "elapsed time: " << std::endl;
    cout << "with PingPong: " << pingpong << " ms" << std::endl;
    cout << "without PingPong: " << withoutpingpong << " ms" << std::endl;
    cout.close();
    t.clear();
    return 0;
}
