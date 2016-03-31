#include <iostream>
#include <vector>
#include <thread>
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

template <class T>
void pushing(int size, stack<T>& data)
{
    for (int i = 0; i < size; ++i)
        data.push(rand() % 10);
}

template <class T>
void poping(int size, stack<T>& data, std::vector<T>* output)
{
    for (int i = 0; i < size; ++i) {
        T a;
        if (data.pop(a))
            output->push_back(a);
        else
            --i;
    }
}

template <class T>
std::vector<std::thread*> pushingThreads(int threadsNumber, int size, stack<T>& data)
{
    std::vector<std::thread*> threads;
    for (int i = 0; i < threadsNumber; ++i)
        threads.push_back(new std::thread(pushing<T>, size, std::ref(data)));
    return threads;
}

template <class T>
std::vector<std::thread*> popingThreads(int threadsNumber, int size, stack<T>& data, 
            std::vector<std::vector<T>*>& output)
{
    std::vector<std::thread*> threads;
    for (int i = 0; i < threadsNumber; ++i)
        threads.push_back(new std::thread(poping<T>, size,
            std::ref(data), output[i]));
    return threads;
}

template <class T>
bool pushTest(int writeN, int writers)
{
    stack<T> data;
    std::vector<std::thread*> w = pushingThreads<T>(writers, writeN, data);
    for (std::vector<std::thread*>::iterator i = w.begin(); i != w.end(); ++i)
        (*i)->join();
    /*
    if (data.size() != writeN * writers) {
        std::cout << "error push" << std::endl;
        return false;
    }
    */
    return true;
}

template <class T>
bool testing(int writeN, int readN, int writers, int readers)
{
    stack<T> data;
    std::vector<std::vector<T>*> output;
    for (int i = 0; i < readers; ++i)
        output.push_back(new std::vector<int>());

    std::vector<std::thread*> w = pushingThreads<T>(writers, writeN, data);
    std::vector<std::thread*> r = popingThreads<T>(readers, readN, data, output);
    for (std::vector<std::thread*>::iterator i = w.begin(); i != w.end(); ++i)
        (*i)->join();
    for (std::vector<std::thread*>::iterator i = r.begin(); i != r.end(); ++i)
        (*i)->join();

    //std::cout << "In CSyncQueue:" << data.size() << std::endl;
    //std::cout << "output: ";
    int sum = 0;
    for (int i = 0; i < readers; ++i)
        sum += output[i]->size();
    std::cout << "sum = " << sum << std::endl;
    bool res = true;
    if (sum != readers * readN) {
        std::cout << "error read in popNoWait" << std::endl;
        res = false;
    }
    /*
    if (sum + data.size() != writeN * writers) {
        std::cout << "error read & write together in popNoWait" << std::endl;
        res = false;
    }
    */
    for (int i = 0; i < output.size(); ++i)
        delete output[i];
    for (int i = 0; i < w.size(); ++i)
        delete w[i];
    for (int i = 0; i < r.size(); ++i)
        delete r[i];
    return res;
}

template <class T>
void test()
{
    int all = 1e5;
    int writers = std::thread::hardware_concurrency();
    int writeN = all / writers;
    if (!pushTest<T>(writeN, writers)) {
        std::cout << "error testing push" << std::endl;
        return;
    }
    std::cout << "push is ok" << std::endl;

    int readers = writers / 2;
    int readN = all / readers;
    //2 times less read
    if (!testing<T>(writeN, readN, writers, readers)) {
        std::cout << "error testing \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
    std::cout << "\ntest with read threads = " << readers << " and write threads = " << writers <<
        " is OK" << std::endl;
    readers = writers;
    readN = all / readers;
    //equal
    if (!testing<T>(writeN, readN, writers, readers)) {
        std::cout << "error testing\n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "OK" << std::endl;
    std::cout << "\ntest with read threads = " << readers << " and write threads = " << writers <<
        " is OK" << std::endl;
    readers = 2 * writers;
    readN = all / readers;
    //2 times more read
    if (!testing<T>(writeN, readN, writers, readers)) {
        std::cout << "error testing \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << std::endl << "Global OK!" << std::endl << std::endl;
}                           

int main()
{
    for (int i = 0; i < 10; ++i)
        test<int>();
    system("pause");
    return 0;
}