#include <future>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <optional>

#pragma promise implementation
template<typename... TArgs> using Action = std::function<void(TArgs...)>;

template<typename T> class TaskState : public std::enable_shared_from_this<TaskState<T>>
{
public:
    
    static std::shared_ptr<TaskState<T>> Create()
    {
        return std::make_shared<TaskState<T>>();
    }

    bool IsComplete()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mIsComplete;
    }
    
    const T &Wait()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        while (!mIsComplete)
        {
            mCondition.wait(lock);
        }
        
        return *mData;
    }
    
    void RaiseOnComplete()
    {
        // set completion flag to true, this is done after all handlers have been invoked
        {
            std::lock_guard<std::mutex> lock(mMutex);
            // set complete flag to true
            mIsComplete = true;
            mCondition.notify_all();
            
            if(mOnComplete)
            {
                mOnComplete(*mData);
            }
        }
    }
    
    void SetResult(const T &result)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mData = result;
        }

        RaiseOnComplete();
    }
    
    void Then(Action<const T &> callback)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mOnComplete = callback;
        }
    }
private:
    std::optional<T> mData;
    bool mIsComplete = false;
    std::mutex mMutex;
    std::condition_variable mCondition;
    Action<const T&> mOnComplete;
};

template<typename T> class Future
{
public:
    using State = TaskState<T>;
    using FutureType = Future<T>;
    template<typename U> friend class Promise;
    
    explicit Future(std::shared_ptr<State> state) : mState(state) {}
    
    virtual bool IsComplete() const
    {
        return mState->IsComplete();
    }
    
    const T &Wait() const
    {
        return mState->Wait();
    }
    
    void Then(Action<const T &> callback)
    {
        mState->Then(callback);
    }
    
private:
    std::shared_ptr<State> mState;
};

template<typename T> class Promise
{
    using State = TaskState<T>;
    using FutureType = Future<T>;
    using PromiseType = Promise<T>;
    template<typename U> friend class Future;

public:
    Promise()
    {
        mState = TaskState<T>::Create();
    }
    
    void SetResult(const T &result) const
    {
        mState->SetResult(result);
    }
    
    FutureType GetFuture() const
    {
        auto retVal = FutureType(mState);
        return retVal;
    }
    
private:
    std::shared_ptr<State> mState;
};


int main() {
    Promise<int> pr;
    
    Future<int> f1 = pr.GetFuture();
    
    f1.Then([](int result)
        {
            std::cout<<" Future completed "<<result<<std::endl;
        });

    std::cout <<"Result from wait "<< f1.Wait() << std::endl;
    return 0;
}
