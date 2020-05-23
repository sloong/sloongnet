#include <queue>
#include <memory>
#include <shared_mutex>
using std::queue;
using std::shared_mutex;
using std::shared_lock;
using std::unique_lock;
using std::shared_ptr;
namespace Sloong
{
    template<typename T>
    class queue_ex : public queue<T>
    {
    private:
        shared_mutex m_mut;
    public:
        queue_ex() {}
        queue_ex(const queue_ex&) = delete;
        void push(T data)
        {
            unique_lock<shared_mutex> lock(m_mut);
            queue<T>::push(data);
        }
        T pop()
        {
            unique_lock<shared_mutex> lock(m_mut);
            auto item = this->front();
            queue<T>::pop();
            return item;
        }
        shared_ptr<T> pops()
        {
            unique_lock<shared_mutex> lock(m_mut);
            shared_ptr<T> res(make_shared<T>(this->front()));
            queue<T>::pop();
            return res;
        }
        bool TryPop(T& t)
        {
            unique_lock<shared_mutex> lock(m_mut);
            if (queue<T>::empty())
                return false;

            t = queue<T>::front();
            queue<T>::pop();
            return true;
        }

        shared_ptr<T> TryPop()
        {
            unique_lock<shared_mutex> lock(m_mut);
            if (queue<T>::empty())
                return shared_ptr<T>();
            shared_ptr<T> res(make_shared<T>(queue<T>::front()));
            queue<T>::pop();
            return res;
        }

        inline bool empty()
        {
            shared_lock<shared_mutex> lock(m_mut);
            return queue<T>::empty();
        }

        inline int size(){
            shared_lock<shared_mutex> lock(m_mut);
            return queue<T>::size();
        }
    };
}
