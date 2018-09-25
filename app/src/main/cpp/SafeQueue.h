//
// create by 李雨晴 on 2018/9/9.
//

#ifndef PROJECTS_SAFEQUEUE_H
#define PROJECTS_SAFEQUEUE_H

#include <queue>
#include <pthread.h>


using namespace std;


template <typename T>
class SafeQueue{

    typedef void(*ReleaseCallback)(T *);
    typedef void(*SyncHandle)(queue<T> &);

public:
    SafeQueue(){
        pthread_mutex_init(&mutex,0);
        pthread_cond_init(&cond,0);
    }


    void push(T value){
        pthread_mutex_lock(&mutex);
        if (work){
            q.push(value);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }

        pthread_mutex_unlock(&mutex);
    }


    //传递指针的引用，可以改变外面指针的值
    int pop(T& value){
        int ret=0;
        pthread_mutex_lock(&mutex);
        //在多核处理器下，由于竞争可能虚假唤醒，
        while (work && q.empty()){
            pthread_cond_wait(&cond,&mutex);
        }
        if (!q.empty()){
            value=q.front();
            q.pop();
            ret=1;
        }

        pthread_mutex_unlock(&mutex);

        return ret;
    }

    void setWork(int work){
        pthread_mutex_lock(&mutex);
        this->work=work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    int empty() {
        return q.empty();
    }

    int  size(){
        return q.size();
    }

    void clear(){
        pthread_mutex_lock(&mutex);
        int size=q.size();
        for (int i = 0; i < size; ++i) {
            T value=q.front();
            //释放value
            releaseCallback(&value);
            q.pop();
        }

        pthread_mutex_unlock(&mutex);
    }

    void sync(){
        pthread_mutex_lock(&mutex);

        //同步代码，当我们调用sync方法时候，能够保证是在同步块中操作queue队列
        syncHandle(q);
        pthread_mutex_unlock(&mutex);

    }

    ~SafeQueue(){
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void setReleaseCallback(ReleaseCallback r){
        this->releaseCallback=r;
    }

    void setSyncHandle(SyncHandle s) {
        syncHandle = s;
    }

private:
    queue<T> q;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    //标记是否工作，1：工作，0：不工作
    int work;
    ReleaseCallback  releaseCallback;
    SyncHandle  syncHandle;
};

#endif //PROJECTS_SAFEQUEUE_H
