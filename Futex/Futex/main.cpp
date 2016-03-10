#include <stdio.h>
#include <thread>
#include <atomic>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <mutex>

class futex {
public:
    futex();
    bool lock();
    bool unlock();
    futex(const futex&) = delete;

private:
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
futex::futex()
{
    ownerId = -1;
}

bool futex::lock()
{
    int desired = -1;
    int idHash = std::this_thread::get_id().hash();
    while (!ownerId.compare_exchange_strong(desired, idHash)) {
        desired = -1;
    }
    return true;
}

bool futex::unlock()
{
    int idHash = std::this_thread::get_id().hash();
    int desired = idHash;
    while (!ownerId.compare_exchange_strong(desired, -1)) {
        desired = idHash;
    }
    return true;
}

std::mutex locking;
//futex locking;

int ans = 0;
int maximum = 1e7;

int inc(int & x)
{
    while (true) {
        locking.lock();
        if (ans >= maximum) {
            locking.unlock();
            break;
        }
        ++ans;
        ++x;
        locking.unlock();
    }
    return ans;
}

int main()
{
    int threadsNumber;
    for (int k = 0; k < 6; ++k) {
        ans = 0;
        maximum = 2 * 1e6;
        if (k > 2)
            maximum = 1e7;
        if (k % 3 == 0)
            threadsNumber = std::thread::hardware_concurrency();
        else
            if (k % 3 == 1)
                threadsNumber = std::thread::hardware_concurrency() / 2;
            else
                threadsNumber = std::thread::hardware_concurrency() * 2;
        std::vector<std::thread *> t;
        std::vector<int> each;
        each.resize(threadsNumber, 0);
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        for (int i = 0; i < threadsNumber; ++i) {
            t.push_back(new std::thread(inc, std::ref(each[i])));
        }
        for (int i = 0; i < threadsNumber; ++i) {
            t[i]->join();
        }
        int sum = 0;
        end = std::chrono::system_clock::now();
        for (int i = 0; i < threadsNumber; ++i) {
            std::cout << each[i] << ' ';
            sum += each[i];
        }
        std::cout << std::endl << sum << ' ' << ans << std::endl;

        int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>
            (end - start).count();
        std::cout << "elapsed time: " << elapsed_seconds << "ms\n";
    }
    system("pause");
    return 0;
}

