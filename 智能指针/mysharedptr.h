#ifndef MYSHAREDPTR_H
#define MYSHAREDPTR_H

#include <iostream>
#include <string>

using namespace std;

template <typename T>

class MySharedPtr
{
private:
    T* ptr;
    long *countRef;

protected:
    void release(void)
    {
        (*countRef) --;
        if(*countRef == 0) {
            if(ptr != NULL) {
                delete ptr;
            }

            delete countRef;
        }
    }

public:
    explicit MySharedPtr(T* _ptr = NULL) : ptr(_ptr), countRef(new long(1))
    {
        ;
    }

    MySharedPtr(const MySharedPtr<T> &other) : ptr(other.ptr), countRef(other.countRef)
    {
        (*countRef) ++;
    }

    ~MySharedPtr()
    {
        release();
    }

    MySharedPtr<T> operator =(const MySharedPtr<T> &other)
    {
        if(this != &other) {
            release();
            ptr = other.ptr;
            countRef = other.countRef;
            (*countRef) ++;
        }
    }

    T& operator *(void)
    {
        return *ptr;
    }

    T* operator ->(void)
    {
        return ptr;
    }

    T* getPtr(void)
    {
        return ptr;
    }

    int getCounRef(void)
    {
        return *countRef;
    }
};
#endif // MYSHAREDPTR_H
