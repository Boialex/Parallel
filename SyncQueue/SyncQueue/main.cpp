#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <list>
#include <stack>
#include <queue>
#include <thread>
#include "CSyncQueue.h"

template <class ARRAY>
void pushing(int size, CSyncContainer<ARRAY>& data)
{
    for (int i = 0; i < size; ++i)
        data.push(rand() % size);
}

template <class ARRAY>
void popWaitTest(int size, CSyncContainer<ARRAY>& data, std::vector<typename ARRAY::value_type>* output)
{
    for (int i = 0; i < size; ++i) {
        ARRAY::value_type a;
        if (data.popOrWait(a))
            output->push_back(a);
        else
            return;
    }
}

template <class ARRAY>
void popNoWaitTest(int size, CSyncContainer<ARRAY>& data, std::vector<typename ARRAY::value_type>* output)
{
    for (int i = 0; i < size; ++i)
    {
        ARRAY::value_type a;
        if (data.popNoWait(a))
            output->push_back(a);
        else
            --i;
    }
}

template <class ARRAY>
std::vector<std::thread*> pushingThreads(int threadsNumber, int size, CSyncContainer<ARRAY>& data)
{
    std::vector<std::thread*> threads;
    for (int i = 0; i < threadsNumber; ++i)
        threads.push_back(new std::thread(pushing<ARRAY>, size, std::ref(data)));
    return threads;
}

template <class ARRAY>
std::vector<std::thread*> popNoWaitThreads(int threadsNumber, int size, CSyncContainer<ARRAY>& data, 
                                        std::vector<std::vector<typename ARRAY::value_type>*>& output)
{
    std::vector<std::thread*> threads;
    for (int i = 0; i < threadsNumber; ++i)
        threads.push_back(new std::thread(popNoWaitTest<ARRAY>, size, 
                              std::ref(data), output[i]));
    return threads;
}

template <class ARRAY>
std::vector<std::thread*> popWaitThreads(int threadsNumber, int size, CSyncContainer<ARRAY>& data,
                                        std::vector<std::vector<typename ARRAY::value_type>*>& output)
{
    std::vector<std::thread*> threads;
    for (int i = 0; i < threadsNumber; ++i)
        threads.push_back(new std::thread(popWaitTest<ARRAY>, size, 
                              std::ref(data), output[i]));
    return threads;
}

template <class ARRAY>
bool pushTest(int writeN, int writers)
{
    CSyncContainer<ARRAY> data;
    std::vector<std::thread*> w = pushingThreads<ARRAY>(writers,
        writeN, data);
    for (std::vector<std::thread*>::iterator i = w.begin(); i != w.end(); ++i)
        (*i)->join();
    if (data.size() != writeN * writers) {
        std::cout << "error push" << std::endl;
        return false;
    }
    return true;
}

template <class ARRAY>
bool testWait(int writeN, int readN, int writers, int readers)
{
    CSyncContainer<ARRAY> data;
    std::vector<std::vector<int>*> output;
    for (int i = 0; i < readers; ++i)
        output.push_back(new std::vector<int>());
    
    std::vector<std::thread*> w = pushingThreads<ARRAY>(writers, writeN, data);
    std::vector<std::thread*> r = popWaitThreads<ARRAY>(readers, readN, data, output);
    for (std::vector<std::thread*>::iterator i = w.begin(); i != w.end(); ++i)
        (*i)->join();
    data.finalize();
    for (std::vector<std::thread*>::iterator i = r.begin(); i != r.end(); ++i)
        (*i)->join();
    
    //std::cout << "In CSyncQueue:" << data.size() << std::endl;
    //std::cout << "output: ";
    int sum = 0;
    for (int i = 0; i < readers; ++i)
        sum += output[i]->size();
    //std::cout << "sum = " << sum << std::endl;
    if (sum != readers * readN) {
        std::cout << "error read in popOrWait" << std::endl;
        for (int i = 0; i < output.size(); ++i)
            delete output[i];
        for (int i = 0; i < w.size(); ++i)
            delete w[i];
        for (int i = 0; i < r.size(); ++i)
            delete r[i];
        return false;
    }
    if (sum + data.size() != writeN * writers) {
        std::cout << "error read & write together in popOrWait" << std::endl;
        for (int i = 0; i < output.size(); ++i)
            delete output[i];
        for (int i = 0; i < w.size(); ++i)
            delete w[i];
        for (int i = 0; i < r.size(); ++i)
            delete r[i];
        return false;
    }
    for (int i = 0; i < output.size(); ++i)
        delete output[i];
    for (int i = 0; i < w.size(); ++i)
        delete w[i];
    for (int i = 0; i < r.size(); ++i)
        delete r[i];
    return true;
}

template <class ARRAY>
bool testNoWait(int writeN, int readN, int writers, int readers)
{
    CSyncContainer<ARRAY> data;
    std::vector<std::vector<int>*> output;
    for (int i = 0; i < readers; ++i)
        output.push_back(new std::vector<int>());
    
    std::vector<std::thread*> w = pushingThreads<ARRAY>(writers,
        writeN, data);
    std::vector<std::thread*> r = popNoWaitThreads<ARRAY>(readers,
        readN, data, output);
    for (std::vector<std::thread*>::iterator i = w.begin(); i != w.end(); ++i)
        (*i)->join();
    for (std::vector<std::thread*>::iterator i = r.begin(); i != r.end(); ++i)
        (*i)->join();

    //std::cout << "In CSyncQueue:" << data.size() << std::endl;
    //std::cout << "output: ";
    int sum = 0;
    for (int i = 0; i < readers; ++i)
        sum += output[i]->size();
    //std::cout << "sum = " << sum << std::endl;
    if (sum != readers * readN) {
        std::cout << "error read in popNoWait" << std::endl;
        for (int i = 0; i < output.size(); ++i)
            delete output[i];
        for (int i = 0; i < w.size(); ++i)
            delete w[i];
        for (int i = 0; i < r.size(); ++i)
            delete r[i];
        return false;
    }
    if (sum + data.size() != writeN * writers) {
        std::cout << "error read & write together in popNoWait" << std::endl;
        for (int i = 0; i < output.size(); ++i)
            delete output[i];
        for (int i = 0; i < w.size(); ++i)
            delete w[i];
        for (int i = 0; i < r.size(); ++i)
            delete r[i];
        return false;
    }
    for (int i = 0; i < output.size(); ++i)
        delete output[i];
    for (int i = 0; i < w.size(); ++i)
        delete w[i];
    for (int i = 0; i < r.size(); ++i)
        delete r[i];
    return true;
}

template <class ARRAY>
bool testBoth(int writeN, int readN, int writers, int readers)
{
    //return true;
    CSyncContainer<ARRAY> data;
    std::vector<std::vector<int>*> output1;
    for (int i = 0; i < readers; ++i)
        output1.push_back(new std::vector<int>());
    std::vector<std::vector<int>*> output2;
    for (int i = 0; i < readers; ++i)
        output2.push_back(new std::vector<int>());


    std::vector<std::thread*> w = pushingThreads<ARRAY>(writers, writeN, data);
    std::vector<std::thread*> r1 = popWaitThreads<ARRAY>(readers, readN / 2, data, output1);
    std::vector<std::thread*> r2 = popNoWaitThreads<ARRAY>(readers, readN / 2, data, output2);
    for (std::vector<std::thread*>::iterator i = w.begin(); i != w.end(); ++i)
        (*i)->join();
    data.finalize();
    for (std::vector<std::thread*>::iterator i = r1.begin(); i != r1.end(); ++i)
        (*i)->join();
    for (std::vector<std::thread*>::iterator i = r2.begin(); i != r2.end(); ++i)
        (*i)->join();

    //std::cout << "In CSyncQueue:" << data.size() << std::endl;
    //std::cout << "output: ";
    int sum = 0;
    for (int i = 0; i < readers; ++i)
        sum += output1[i]->size();
    for (int i = 0; i < readers; ++i)
        sum += output2[i]->size();
    //std::cout << "sum = " << sum << std::endl;
    bool f = true;
    if (sum != readers * readN) {
        std::cout << "error read in popOrWait" << std::endl;
        f = false;
    }
    if (sum + data.size() != writeN * writers) {
        std::cout << "error read & write together in popOrWait" << std::endl;
        f = false;
    }
    for (int i = 0; i < output1.size(); ++i)
        delete output1[i];
    for (int i = 0; i < output2.size(); ++i)
        delete output2[i];
    for (int i = 0; i < w.size(); ++i)
        delete w[i];
    for (int i = 0; i < r1.size(); ++i)
        delete r1[i];
    for (int i = 0; i < r2.size(); ++i)
        delete r2[i];
    return f;
}

template <class ARRAY>
void test()
{
    int all = 1e5;
    int writers = std::thread::hardware_concurrency();
    int writeN = all / writers;
    if (!pushTest<ARRAY>(writeN, writers)) {
        std::cout << "error testing push" << std::endl;
        return;
    }
    std::cout << "push is ok" << std::endl;

    int readers = writers / 2;
    int readN = all / readers;
    //2 times less read
    if (!testWait<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popOrWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popOrWait is OK" << std::endl;
    if (!testNoWait<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popNoWait \n read threads: " << 
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popNoWait is OK" << std::endl;
    if (!testBoth<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popNoWait & popWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popOrWait & popNoWait is OK" << std::endl;
    std::cout << "\ntest with read threads = " << readers << " and write threads = " << writers <<
        " is OK" << std::endl;
    readers = writers;
    readN = all / readers;
    //equal
    if (!testNoWait<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popNoWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popNoWait is OK" << std::endl;
    if (!testWait<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popOrWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popOrWait is OK" << std::endl;
    if (!testBoth<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popNoWait & popWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popOrWait & popNoWait is OK" << std::endl;
    std::cout << "\ntest with read threads = " << readers << " and write threads = " << writers <<
        " is OK" << std::endl;
    readers = writers * 2;
    readN = all / readers;
    //2 times more read
    if (!testNoWait<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popNoWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popNoWait is OK" << std::endl;
    if (!testWait<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popOrWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popOrWait is OK" << std::endl;
    if (!testBoth<ARRAY>(writeN, readN, writers, readers)) {
        std::cout << "error testing popNoWait & popWait \n read threads: " <<
            readers << "\n write threads: " << writers << std::endl;
        return;
    }
    std::cout << "popOrWait & popNoWait is OK" << std::endl;
    std::cout << "\ntest with read threads = " << readers << " and write threads = " << writers <<
        " is OK" << std::endl;
    std::cout << std::endl << "Global OK!" << std::endl << std::endl;
}

int main() 
{
    std::cout << "vector test :" << std::endl;
    test<std::vector<int>>();
    std::cout << "list test :" << std::endl;
    test<std::list<int>>();
    std::cout << "deque test :" << std::endl;
    test<std::deque<int>>();
    std::cout << "queue test :" << std::endl;
    test<std::queue<int>>();
    std::cout << "stack test :" << std::endl;
    test<std::stack<int>>();
    system("pause");
    return 0;
}