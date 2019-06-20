#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include <vector>
#include <queue>
#include <thread>
#include <iostream>
#include <stdexcept>
#include <condition_variable>
#include <memory> //unique_ptr

const int MAX_THREADS = 1000; //����߳���Ŀ

template <typename T>
class threadPool
{
public:
    /*Ĭ�Ͽ�һ���߳�*/
    threadPool(int number = 1);
    ~threadPool();
    /*��������У�task_queue�����������<T *>*/
    bool append(T *request);

private:
    /*�����߳���Ҫ���еĺ���,���ϵĴ����������ȡ����ִ��*/
    static void *worker(void *arg);
    void run();

private:
    std::vector<std::thread> work_threads; /*�����߳�*/
    std::queue<T *> tasks_queue;		   /*�������*/
    std::mutex queue_mutex;
    std::condition_variable condition;  /*������unique_lock���ʹ��*/
    bool stop;
};

template <typename T>
threadPool<T>::threadPool(int number) : stop(false)
{
    if (number <= 0 || number > MAX_THREADS)
        throw std::exception();
    for (int i = 0; i < number; i++)
    {
        std::cout << "������" << i << "���߳� " << std::endl;
        /*
        std::thread temp(worker, this);
        �����ȹ����ٲ���
        */
        work_threads.emplace_back(worker, this);
    }
}
template <typename T>
inline threadPool<T>::~threadPool()
{
    /*�������bug ������Ϊ��д�������shit  */
    //work_threads.clear();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (auto &ww : work_threads)
        ww.join();
}
template <typename T>
bool threadPool<T>::append(T *request)
{
    /*������������ʱһ��Ҫ��������Ϊ���������̹߳���*/
    queue_mutex.lock();
    tasks_queue.push(request);
    queue_mutex.unlock();
    condition.notify_one(); //�̳߳���ӽ�ȥ��������ȻҪ֪ͨ�ȴ����߳�
    return true;
}
template <typename T>
void *threadPool<T>::worker(void *arg)
{
    threadPool *pool = (threadPool *)arg;
    pool->run();
    return pool;
}
template <typename T>
void threadPool<T>::run()
{
    while (!stop)
    {
        std::unique_lock<std::mutex> lk(this->queue_mutex);
        /*��unique_lock() ����������Զ�������*/
        this->condition.wait(lk, [this] { return !this->tasks_queue.empty(); });
        //���������в�Ϊ�գ���ͣ�����ȴ�����
        if (this->tasks_queue.empty())
        {
            continue;
        }
        else
        {
            T *request = tasks_queue.front();
            tasks_queue.pop();
            if (request)
                request->process();
        }
    }
}
#endif
