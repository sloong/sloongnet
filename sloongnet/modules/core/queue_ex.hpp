#include <condition_variable>
#include <queue>
#include <memory>
using std::queue;
using std::condition_variable;
using std::shared_ptr;
namespace Sloong
{
    template<typename T>
    class queue_ex : public queue<T>
    {
    private:
        mutex m_mut;
        condition_variable m_data_cond;
    public:
        queue_ex() {}
        queue_ex(const queue_ex&) = delete;
        void push(T data)
        {
            lock_guard<mutex> lg(m_mut);
            queue<T>::push(data);
            m_data_cond.notify_one();
        }
        T pop()
        {
            lock_guard<mutex> lg(m_mut);
            auto item = this->front();
            queue<T>::pop();
            return item;
        }
        shared_ptr<T> pops()
        {
            lock_guard<mutex> lg(m_mut);
            shared_ptr<T> res(make_shared<T>(this->front()));
            queue<T>::pop();
            return res;
        }
        void WaitPop(T& t)
        {
            unique_lock<mutex> ul(m_mut);
            m_data_cond.wait(ul, [this] {return !queue<T>::empty(); });
            t = queue<T>::front();
            queue<T>::pop();
        }
        shared_ptr<T> WaitPop()
        {
            unique_lock<mutex> ul(m_mut);
            m_data_cond.wait(ul, [this] {return !queue<T>::empty(); });

            shared_ptr<T> res(make_shared<T>(queue<T>::front()));
            queue<T>::pop();
            return res;
        }
        bool TryPop(T& t)
        {
            lock_guard<mutex> lg(m_mut);
            if (queue<T>::empty())
                return false;

            t = queue<T>::front();
            queue<T>::pop();
            return true;
        }

        shared_ptr<T> TryPop()
        {
            lock_guard<mutex> lg(m_mut);
            if (queue<T>::empty())
                return shared_ptr<T>();
            shared_ptr<T> res(make_shared<T>(queue<T>::front()));
            queue<T>::pop();
            return res;
        }

        bool empty_safe()
        {
            lock_guard<mutex> lg(m_mut);
            return queue<T>::empty();
        }

        int size_safe(){
            lock_guard<mutex> lg(m_mut);
            return queue<T>::size();
        }
    };
}
