#include <mutex>
#include <condition_variable>

class readWriteMutex {
public:
    void read_lock()
    {
        std::unique_lock<std::mutex> m(mutex);
        //mutex.lock();
        readCount++;
        while (write)
            cond_var.wait(m);
        m.unlock();
    }

    void write_lock()
    {
        std::unique_lock<std::mutex> m(mutex);
        //mutex.lock();
        while (write || readCount > 0)
            cond_var.wait(m);
        write = true;
        m.unlock();
    }

    void read_unlock() 
    {
        std::unique_lock<std::mutex> m(mutex);
        readCount--;
        if (readCount == 0)
            cond_var.notify_one();
        m.unlock();
    }

    void write_unlock()
    {
        std::unique_lock<std::mutex> m(mutex);
        write = false;
        cond_var.notify_all();
        m.unlock();
    }
private:
    bool write;
    int readCount;
    std::mutex mutex;
    std::condition_variable cond_var;
};

class readWriteMutex {
public:
    void read_lock()
    {
        std::unique_lock<std::mutex> m(mutex);
        while (write || writeCount > 0)
            cond_var.wait(m);
        readCount++;
        m.unlock();
    }

    void write_lock()
    {
        std::unique_lock<std::mutex> m(mutex);
        writeCount++;
        while (write || readCount > 0)
            cond_var.wait(m);
        write = true;
        m.unlock();
    }

    void read_unlock()
    {
        std::unique_lock<std::mutex> m(mutex);
        readCount--;
        if (readCount == 0)
            cond_var.notify_one();
        m.unlock();
    }

    void write_unlock()
    {
        std::unique_lock<std::mutex> m(mutex);
        writeCount--;
        write = false;
        cond_var.notify_all();
        m.unlock();
    }
private:
    bool write;
    int writeCount;
    int readCount;
    std::mutex mutex;
    std::condition_variable cond_var;

};