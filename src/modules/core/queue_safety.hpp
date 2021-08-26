/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-08-26 13:47:32
 * @LastEditTime: 2021-08-26 15:16:44
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/queue_safety.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#pragma once

#include <queue>
#include <memory>
#include <shared_mutex>
using std::queue;
using std::shared_lock;
using std::shared_mutex;
using std::shared_ptr;
using std::unique_lock;
namespace Sloong
{
    template <typename T>
    class queue_safety : public queue<T>
    {
    private:
        shared_mutex m_mut;
        T pop();

    public:
        queue_safety() {}
        queue_safety(const queue_safety &) = delete;
        void push(const T &data)
        {
            unique_lock<shared_mutex> lock(m_mut);
            queue<T>::push(data);
        }
        void push(T &&data)
        {
            unique_lock<shared_mutex> lock(m_mut);
            queue<T>::push(std::move(data));
        }

        T pop(T &&def)
        {
            if (empty())
                return move(def);

            unique_lock<shared_mutex> lock(m_mut);
            auto t = move(queue<T>::front());
            queue<T>::pop();
            return move(t);
        }

        bool take(T *t)
        {
            if (empty() || t == nullptr)
                return false;

            unique_lock<shared_mutex> lock(m_mut);
            *t = move(queue<T>::front());
            queue<T>::pop();
            return true;
        }

        unique_ptr<T> pop_ptr()
        {
            if (empty())
                return nullptr;

            unique_lock<shared_mutex> lock(m_mut);
            unique_ptr<T> res(make_unique<T>(std::move(queue<T>::front())));
            queue<T>::pop();
            return res;
        }

        inline void clear()
        {
            unique_lock<shared_mutex> lock(m_mut);
            while (!queue<T>::empty())
            {
                queue<T>::pop();
            }
        }

        inline bool empty()
        {
            shared_lock<shared_mutex> lock(m_mut);
            return queue<T>::empty();
        }

        inline int size()
        {
            shared_lock<shared_mutex> lock(m_mut);
            return queue<T>::size();
        }
    };
}
