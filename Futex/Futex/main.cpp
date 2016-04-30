#include <stdio.h>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <mutex>

class Futex {
public:
    Futex();
    bool lock(int);
    bool unlock(int);
    Futex(const Futex&) = delete;

private:
    int hash;
    std::atomic<int> ownerId;
};

/* if (x == desired) {
x = newVal;
return true;
} else {
desired = x;
return false;
}
*/
Futex::Futex()
{
    ownerId.store(-1);
    hash = -1;
}

bool Futex::lock(int idHash)
{
    int desired = hash;
    while (!ownerId.compare_exchange_strong(desired, idHash)) {
        desired = hash;
        std::this_thread::yield();
    }
    return true;
}

bool Futex::unlock(int idHash)
{
    int desired = idHash;
    if (!ownerId.compare_exchange_strong(desired, hash)) {
        assert(false);
    }
    return true;
}

std::mutex mlocking;
Futex flocking;

long long ans = 0;
long long maximum = 1e7;

int incFutex(long long & x)
{
    int thisHash = std::hash<std::thread::id>()(std::this_thread::get_id());
    while (true) {
        flocking.lock(thisHash);
        if (ans >= maximum) {
            flocking.unlock(thisHash);
            break;
        }
        ++ans;
        ++x;
        flocking.unlock(thisHash);
    }
    return 0;
}

int incMutex(long long & x)
{
    while (true) {
        mlocking.lock();
        if (ans >= maximum) {
            mlocking.unlock();
            break;
        }
        ++ans;
        ++x;
        mlocking.unlock();
    }
    return 0;
}

void runTestFutex(int threadsNumber, long long limit)
{
    ans = 0;
    maximum = limit;
    std::cout << "N threads = " << threadsNumber << std::endl;
    std::cout << "max = " << limit << std::endl;
    std::vector<std::thread *> t(threadsNumber);
    std::vector<long long> each(threadsNumber, 0);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    for (int i = 0; i < threadsNumber; ++i) {
        t[i] = new std::thread(incFutex, std::ref(each[i]));
    }
    for (int i = 0; i < threadsNumber; ++i) {
        t[i]->join();
    }
    long long sum = 0;
    end = std::chrono::system_clock::now();
    for (int i = 0; i < threadsNumber; ++i) {
        std::cout << each[i] << ' ';
        sum += each[i];
    }
    std::cout << std::endl << sum << ' ' << ans << std::endl;

    long long elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>
        (end - start).count();
    std::cout << "elapsed time: " << elapsed_seconds << "ms" << std::endl;
    std::cout << std::endl;
    t.clear();
}

void runTestMutex(int threadsNumber, long long limit)
{
    ans = 0;
    maximum = limit;
    std::cout << "N threads = " << threadsNumber << std::endl;
    std::cout << "max = " << limit << std::endl;
    std::vector<std::thread *> t(threadsNumber);
    std::vector<long long> each(threadsNumber, 0);
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    for (int i = 0; i < threadsNumber; ++i) {
        t[i] = new std::thread(incMutex, std::ref(each[i]));
    }
    for (int i = 0; i < threadsNumber; ++i) {
        t[i]->join();
    }
    long long sum = 0;
    end = std::chrono::system_clock::now();
    for (int i = 0; i < threadsNumber; ++i) {
        std::cout << each[i] << ' ';
        sum += each[i];
    }
    std::cout << std::endl << sum << ' ' << ans << std::endl;

    long long elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>
        (end - start).count();
    std::cout << "elapsed time: " << elapsed_seconds << "ms" << std::endl;
    std::cout << std::endl;
    t.clear();
}

int main()
{
    int kernelsNumber = std::thread::hardware_concurrency();
    std::cout << "std::thread::hardware_concurrency() = " << kernelsNumber << std::endl;
    std::vector<int> threadsNumber(3);
    threadsNumber[0] = kernelsNumber / 2;
    threadsNumber[1] = kernelsNumber;
    threadsNumber[2] = kernelsNumber * 2;
    for (int k = 0; k < 6; ++k) {
        ans = 0;
        long long limit = (k < 3 ? 1e7 : 5e8);
        std::cout << "ran futex" << std::endl;
        runTestFutex(threadsNumber[k % 3], limit);
        std::cout << "ran mutex" << std::endl;
        runTestMutex(threadsNumber[k % 3], limit);
    }
    
    flocking.lock(std::hash<std::thread::id>()(std::this_thread::get_id()));
    std::cout << std::endl << "main thread test OK" << std::endl;
    flocking.unlock(std::hash<std::thread::id>()(std::this_thread::get_id()));
    system("pause");
    return 0;
}

