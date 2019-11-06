template<typename T> 
class queue_ex
{
private:
    mutable mutex m_mut;
    queue<T> m_queue;
    condition_variable m_data_cond;
public:
    queue_ex() {}
    queue_ex(const queue_ex&) = delete;
    void push(T data)
    {
        lock_guard<mutex> lg(m_mut);
        m_queue.push(data);
        m_data_cond.notify_one();
    }
    T pop()
    {
        lock_guard<mutex> lg(m_mut);
        auto item = m_queue.front();
        m_queue.pop();
        return item;
    }
    shared_ptr<T> pops()
    {
        lock_guard<mutex> lg(m_mut);
        shared_ptr<T> res(make_shared<T>(m_queue.front()));
        m_queue.pop();
        return res;
    }
    void WaitPop(T&t)
    {
        unique_lock<mutex> ul(m_mut);
        m_data_cond.wait(ul, [this] {return !m_queue.empty(); });
        t = m_queue.front();
        m_queue.pop();
    }
    shared_ptr<T> WaitPop()
    {
        unique_lock<mutex> ul(m_mut);
        m_data_cond.wait(ul, [this] {return !m_queue.empty(); });

        shared_ptr<T> res(make_shared<T>(m_queue.front()));
        m_queue.pop();
        return res;
    }
    bool TryPop(T &t)
    {
        lock_guard<mutex> lg(m_mut);
        if (m_queue.empty())
            return false;

        t = m_queue.front();
        m_queue.pop();
        return true;
    }

    shared_ptr<T> TryPop()
    {
        lock_guard<mutex> lg(m_mut);
        if (m_queue.empty())
            return shared_ptr<T>();
        shared_ptr<T> res(make_shared<T>(m_queue.front()));
        m_queue.pop();
        return res;
    }

    bool empty()
    {
        lock_guard<mutex> lg(m_mut);
        return m_queue.empty();
    }
};