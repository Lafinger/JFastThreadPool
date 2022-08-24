#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>


namespace Jason
{

class FastThreadPool
{

private:
    struct Utilities
    {
        bool is_shutdown_ = false;
        std::mutex mtx;
        std::condition_variable cv;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Utilities> util_;

public:
    template <typename Fn, typename ...Args>
    inline void Execute(Fn&& task, Args&& ...args) 
    {
        if constexpr (0 == sizeof...(args))
        {
            // lock
            {
                std::lock_guard<std::mutex> u_lock(util_->mtx);
                util_->tasks.emplace(std::forward<Fn>(task));
            }
        }
        else
        {
            // lock
            {
                std::lock_guard<std::mutex> u_lock(util_->mtx);
                util_->tasks.emplace([&](){task(args...);});
            }
        }
        util_->cv.notify_one();
    }

    explicit FastThreadPool(size_t thread_count) : util_(std::make_shared<Utilities>())
    {
        for (size_t i = 0; i < thread_count; ++i)
        {
            // lambda
            std::thread([this] {
                
                while(1)
                {
                    // lock
                    {
                        std::unique_lock<std::mutex> u_lock(this->util_->mtx);
                        this->util_->cv.wait(u_lock, [this]() {
                                return this->util_->is_shutdown_ || !this->util_->tasks.empty(); 
                            });

                        if(this->util_->is_shutdown_ && this->util_->tasks.empty()) {
                            return;
                        }

                        auto task = std::move(this->util_->tasks.front());
                        this->util_->tasks.pop();
                        task();
                    }
                    
                }
            }).detach();
        }
    }

    FastThreadPool() = default;
    FastThreadPool(FastThreadPool&&) = default;
    ~FastThreadPool()
    {
        if ((bool)util_)
        {
            // lock
            {
                std::lock_guard<std::mutex> u_lock(util_->mtx);
                util_->is_shutdown_ = true;
            }
            util_->cv.notify_all();
        }
    }

}; // class FastThreadPool

} // namespace Jason