#include <thread>
#include <mutex>
#include <condition_variable>

template<typename ARRAY>
struct isContainer {
private:
    static int check(...);
    template<typename T> 
    static decltype(std::declval<T>().pop()) check(const T&);
public:
    static constexpr 
        bool value = !(std::is_same<void, decltype(check(std::declval<ARRAY>()))>::value);
};

template<typename ARRAY>
struct isStack {
private:
    static void check(...);
    template<typename T> 
    static decltype(std::declval<T>().top()) check(const T&);
public:
    static constexpr 
        bool value = !(std::is_same<void, decltype(check(std::declval<ARRAY>()))>::value);
};

template <class ARRAY>
class CSyncContainer {
    typedef typename ARRAY::value_type value_type;
public:
    CSyncContainer() : final(false) {};
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
        if (data.empty())
        {
            locker.unlock();
            return false;
        }
        item = std::move(getData(typeOfArray<isContainer<ARRAY>::value, isStack<ARRAY>::value>()));
        popData(typeOfArray<isContainer<ARRAY>::value, isStack<ARRAY>::value>());
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
        item = std::move(getData(typeOfArray<isContainer<ARRAY>::value, isStack<ARRAY>::value>()));
        popData(typeOfArray<isContainer<ARRAY>::value, isStack<ARRAY>::value>());
        return true;
    }

    void push(const value_type& item)
    {
        std::unique_lock<std::mutex> locker(mutex);
        pushData(typeOfArray<isContainer<ARRAY>::value, isStack<ARRAY>::value>(), item);
        locker.unlock();
        cond_var.notify_one();
    }

    void push(value_type&& item)
    {
        std::unique_lock<std::mutex> locker(mutex);
        pushData(typeOfArray<isContainer<ARRAY>::value, isStack<ARRAY>::value>(), std::move(item));
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

    template<bool isContainer, bool isStack>
    struct typeOfArray {};

    template<bool isContainer, bool isStack>
    value_type& getData(typeOfArray<isContainer, isStack>);
    value_type& getData(typeOfArray<true, false>) {return data.back();}
    value_type& getData(typeOfArray<false, false>) {return data.back();}
    value_type& getData(typeOfArray<false, true>) {return data.top();}

    template<bool isContainer, bool isStack>
    void popData(typeOfArray<isContainer, isStack>);
    void popData(typeOfArray<true, false>) {data.pop_back();}
    void popData(typeOfArray<false, false>) {data.pop();}
    void popData(typeOfArray<false, true>) {data.pop();}

    template<bool isContainer, bool isStack>
    void pushData(typeOfArray<isContainer, isStack>, const value_type& item);
    void pushData(typeOfArray<true, false>, const value_type& time) {data.push_back(item);}
    void pushData(typeOfArray<false, false>, const value_type& item) {data.push(item);}
    void pushData(typeOfArray<false, true>, const value_type& item) {data.push(item);}

    template<bool isContainer, bool isStack>
    void pushData(typeOfArray<isContainer, isStack>, value_type&& item);
    void pushData(typeOfArray<true, false>, value_type&& item) {data.push_back(std::move(item));}
    void pushData(typeOfArray<false, false>, value_type&& item) {data.push(std::move(item));}
    void pushData(typeOfArray<false, true>, value_type&& item) {data.push(std::move(item));}
};