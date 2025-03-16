#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

template <typename T, int N>
class avec
{
public:
    avec();
private:
    T data[N];
    int size = 0;
public:
    T& operator [] (const int& index) { return data[index]; };
public:
    void add(const T& element);
    void pop();
    void clear();
    void erase(int index);
    void insert(const T& element, int index);
public:
    constexpr int get_size();
    constexpr int get_capacity();
public:
    T* iter_begin();
    T* iter_end();
};

template <typename T, int N>
inline avec<T, N>::avec()
{
    this->size = 0;
}

template <typename T, int N>
inline void avec<T, N>::add(const T& element)
{
    if (this->size >= N) {
        return;
    }
    ++this->size;
    this->data[this->size - 1] = element;
}

template <typename T, int N>
inline void avec<T, N>::pop()
{
    --this->size;
    this->size = std::max(size, 0);
}

template <typename T, int N>
inline void avec<T, N>::clear()
{
    this->size = 0;
}

template <typename T, int N>
inline void avec<T, N>::erase(int index)
{
    if (this->size <= 0 || index < 0 || index >= this->size) return;
    for (int i = index; i < this->size - 1; ++i) {
        this->data[i] = this->data[i + 1];
    }
    pop();
}

template <typename T, int N>
inline void avec<T, N>::insert(const T& element, int index)
{
    if (this->size >= N || index < 0 || index >= this->size) return;
    for (int i = this->size; i > index; --i) {
        this->data[i] = this->data[i - 1];
    }
    this->data[index] = element;
    ++this->size;
}

template <typename T, int N>
constexpr int avec<T, N>::get_size()
{
    return this->size;
}

template <typename T, int N>
constexpr int avec<T, N>::get_capacity()
{
    return N;
}

template <typename T, int N>
inline T* avec<T, N>::iter_begin()
{
    return this->data;
}

template <typename T, int N>
inline T* avec<T, N>::iter_end()
{
    return this->data + this->size;
}