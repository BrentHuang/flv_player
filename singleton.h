#ifndef SINGLETON_H
#define SINGLETON_H

// 可以把任何类包装成线程安全的全局单例类，出口默认智能指针

#include <mutex>
#include <memory>

template<typename T>
class Singleton
{
public:
    // 获取全局单例对象
    template<typename ...Args>
    static std::shared_ptr<T> Instance(Args&& ... args)
    {
        if (nullptr == sington_)
        {
            std::lock_guard<std::mutex> guard(mutex_);
            if (nullptr == sington_)
            {
                sington_ = std::make_shared<T>(std::forward<Args>(args)...);
            }
        }

        return sington_;
    }

    // 主动析构单例对象（一般不需要主动析构，除非特殊需求）
    static void Release()
    {
        if (sington_ != nullptr)
        {
            sington_.reset();
            sington_ = nullptr;
        }
    }

private:
    explicit Singleton();
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    ~Singleton();

private:
    static std::shared_ptr<T> sington_;
    static std::mutex mutex_;
};

template<typename T>
std::shared_ptr<T> Singleton<T>::sington_ = nullptr;

template<typename T>
std::mutex Singleton<T>::mutex_;

///////////////////////////////////////////////////////////////////////////////
// 使用示例
//class MyClass
//{
//public:
//    MyClass(int a) : a_(a)
//    {
//        std::cout << data_.data() << " " << a_ << std::endl;
//    }

//    MyClass(const std::string& data) : data_(data)
//    {
//        std::cout << data_.data() << " " << a_ << std::endl;
//    }

//    ~MyClass()
//    {
//        std::cout << "destory" << std::endl;
//    }

//private:
//    std::string data_;
//    int a_;
//};


//int main(int argc, char* argv[])
//{
//    auto p1 = Singleton<MyClass>::Instance("create");
//    auto p2 = Singleton<MyClass>::Instance(1);
//    assert(p1 == p2);
//    return 0;
//}

#endif // SINGLETON_H
