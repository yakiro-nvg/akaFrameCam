/* Copyright (c) 2021 FPT Software - All Rights Reserved. Proprietary.
 * Unauthorized copying of this file, via any medium is strictly prohibited. */
namespace akaFrame { namespace cam {

template <typename T>
Array<T>::Array(mem::Allocator &allocator)
        : _allocator(allocator)
        , _size(0)
        , _capacity(0)
        , _items(nullptr)
{
        // nop
}

template <typename T>
Array<T>::Array(const Array<T> &other)
        : _allocator(other._allocator)
        , _size(0)
        , _capacity(0)
        , _items(nullptr)
{
        *this = other;
}

template <typename T>
Array<T>& Array<T>::operator=(const Array<T> &other)
{
        array::resize(*this, other._size);
        memcpy(_items, other._items, sizeof(T) * other._size);
        return *this;
}

template <typename T>
Array<T>::~Array(void)
{
        _allocator.deallocate(_items);
}

template <typename T>
Array<T>::operator bool(void) const
{
        return _size != 0;
}

template <typename T>
T& Array<T>::operator[](i64 i)
{
        return _items[i];
}

template <typename T>
const T& Array<T>::operator[](i64 i) const
{
        return _items[i];
}

namespace array {

template <typename T>
i64 size(const Array<T> &a)
{
        return a._size;
}

template <typename T>
void reserve(Array<T> &a, i64 capacity)
{
        if (capacity == 0 || a._capacity >= capacity) return;

        const i64 dsize = a._allocator.divisor_size();
        const i64 required_bytes = sizeof(T) * capacity;
        const i64 num_pages = (required_bytes + dsize) / dsize;
        const i64 new_bytes = num_pages * dsize;
        T *const new_items = (T*)a._allocator.allocate(new_bytes);
        memcpy(new_items, a._items, sizeof(T) * a._size);
        a._allocator.deallocate(a._items);
        a._capacity = new_bytes / sizeof(T);
        a._items = new_items;
}

template <typename T>
void resize(Array<T> &a, i64 new_size)
{
        if (new_size > a._capacity) {
                reserve(a, new_size);
        }

        a._size = new_size;
}

template <typename T>
void push(Array<T> &a, const T &item)
{
        if (a._size + 1 > a._capacity) {
                reserve(a, a._size + 1);
        }

        push_unsafe(a, item);
}

template <typename T>
T& push(Array<T> &a)
{
        if (a._size + 1 > a._capacity) {
                reserve(a, a._size + 1);
        }

        return push_unsafe(a);
}

template <typename T>
void push_unsafe(Array<T> &a, const T &item)
{
        a._items[a._size++] = item;
}

template <typename T>
T& push_unsafe(Array<T> &a)
{
        return a._items[a._size++];
}

template <typename T>
T* push_many(Array<T> &a, i64 num_items)
{
        const i64 old_size = a._size;
        resize(a, old_size + num_items);
        return a._items + old_size;
}

}}} // namespace akaFrame.cam.array
