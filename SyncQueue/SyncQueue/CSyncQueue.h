#include <thread>
#include <mutex>
#include <condition_variable>

template <class ARRAY>
class CSyncContainer {
    typedef typename ARRAY::value_type value_type;
public:
    CSyncContainer() : data(0), final(false) {};
    CSyncContainer(int size) : data(size), final(false) {};
    /*
    value_type& popOrWait()
    {
        std::unique_lock<std::mutex> locker(mutex);
        while (data.empty() && !final) {
            cond_var.wait(locker);
        }
        if (data.empty())
            return;
        value_type item = data.front();
        data.pop_front();
        return item;
    }
    */
    bool popNoWait(value_type& item)
    {
        std::unique_lock<std::mutex> locker(mutex);
        if (data.empty() || final)
        {
            locker.unlock();
            return false;
        }
        item = std::move(data.front());
        data.pop_back();
        return true;
    }

    bool popOrWait(value_type& item)
    {
        std::unique_lock<std::mutex> locker(mutex);
        while (data.empty() && !final)
        {
            cond_var.wait(locker);
        }
        if (data.empty())
            return false;
        item = std::move(data.front());
        data.pop_back();
        return true;
    }

    void push(const value_type& item)
    {
        std::unique_lock<std::mutex> locker(mutex);
        data.push_back(item);
        locker.unlock();
        cond_var.notify_one();
    }

    void push(value_type&& item)
    {
        std::unique_lock<std::mutex> locker(mutex);
        data.push_back(std::move(item));
        locker.unlock();
        cond_var.notify_one();
    }

    void finalize()
    {
        final = true;
        cond_var.notify_all();
    }

    int size()
    {
        return data.size();
    }

private:
    ARRAY data;
    std::mutex mutex;
    std::condition_variable cond_var;
    bool final = false;
};