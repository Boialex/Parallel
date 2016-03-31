#include <atomic>

template <class T>
struct Node{
    Node(const T& _data) : next(NULL), data(_data) {};
    Node(const T&& _data) : next(NULL), data(std::move(_data)) {};
    Node * next;
    T data;
};

template <class T>
class stack{
    std::atomic<Node<T>*> HEAD;
public:
    void push(const T & _data);
    bool pop(T& _data);
};

template <class T>
void stack<T>::push(const T& _data)
{
    Node<T> * d = new Node<T>(_data);
    d->next = HEAD.load();
    while (!HEAD.compare_exchange_strong(d->next, d));
}

template <class T>
bool stack<T>::pop(T& _data) 
{
    Node<T>* a = HEAD;
    do {
        if (a == 0)
            return false;
    } while (!HEAD.compare_exchange_strong(a, a->next));
    _data = a->data;
    delete a;
    return true;
}