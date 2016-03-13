#include <iostream>
#include <vector>

template <class ARRAY>
class CSyncQueue {
    typedef typename ARRAY::value_type value_type;
    ARRAY data;
public:
    void push(typename ARRAY::value_type&);
};

int main()
{
    CSyncQueue<std::vector<int>> q;
    return 0;
}