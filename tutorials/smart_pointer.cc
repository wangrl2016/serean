//
// Created by wangrl2016 on 2022/9/22.
//

#include <iostream>
#include <utility>

// 1. 简单介绍shared_ptr和unique_ptr指针
// 2. 自定义shared_ptr来理解内存管理
// 代码地址：https://github.com/wangrl2016/serean.git

// RefCnt is the base class for objects that may be shared by multiple
// objects. When an existing owner wants to share a reference, it calls Ref().
// When an owner wants to release its reference, it calls Unref(). When the
// shared object's reference count goes to zero as the result of an Unref()
// call, its (virtual) destructor is called. It is an error for the
// destructor to be called explicitly (or via the object going out of scope on
// the stack or calling delete) if GetRefCnt() > 1.
class RefCnt {
public:
    RefCnt(RefCnt&&) = delete;

    RefCnt(const RefCnt&) = delete;

    RefCnt& operator=(RefCnt&&) = delete;

    RefCnt& operator=(const RefCnt&) = delete;

    // Default construct, initializing the reference count to 1.
    RefCnt() : ref_cnt_(1) {}

    // destruct
    virtual ~RefCnt() = default;

    bool Unique() const {
        if (1 == ref_cnt_.load(std::memory_order_acquire)) {
            // Acquire barrier is only really needed if we return
            // true. It prevents code conditioned on the result of
            // Unique() from running until previous owners are all
            // totally done calling unref().
            return true;
        }
        return false;
    }

    void Ref() const {
        // No barrier required.
        ref_cnt_.fetch_add(+1, std::memory_order_relaxed);
    }

    // Decrement the reference count. If the reference count is 1
    // before the decrement, then delete the object. Note that
    // if this is the case, then the object needs to have been
    // allocated via new, and not on the stack.
    void Unref() const {
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == ref_cnt_.fetch_add(-1, std::memory_order_acq_rel)) {
            // Like Unique(), acquire is only needed on success, to make sure
            // code in InternalDispose() doesn't happen before the decrement.
            this->InternalDispose();
        }
    }

private:
    virtual void InternalDispose() const {
        delete this;
    }

private:
    mutable std::atomic<int32_t> ref_cnt_;
};

// Check if the argument is non-null, and if so, call obj->ref() and return obj.
template<typename T>
static inline T* SafeRef(T* obj) {
    if (obj) {
        obj->Ref();
    }
    return obj;
}

template<typename T>
static inline void SafeUnref(T* obj) {
    if (obj) {
        obj->Unref();
    }
}

// Shared pointer class to wrap classes that support a Ref()/Unref() interface.
template<typename T>
class SharedPtr {
public:
    constexpr SharedPtr() : ptr_(nullptr) {}

    constexpr explicit SharedPtr(std::nullptr_t) : ptr_(nullptr) {}

    // Shares the underlying object by call Ref(), so that both the argument and the newly
    // create SharedPtr both have a reference to it.
    SharedPtr(const SharedPtr& that) : ptr_(SafeRef(that.get())) {}

    // Move the underlying object from the argument to the newly create SharedPtr. Afterwords only
    // the new SharedPtr will have a reference to the object, and the argument will point to null.
    // No call to Ref() or Unref() will be made.
    SharedPtr(SharedPtr<T>&& that) noexcept: ptr_(that.release()) {}

    // Adopt the bare pointer into the newly create SharedPtr.
    // No call to Ref() or Unref() will be made.
    explicit SharedPtr(T* obj) : ptr_(obj) {}

    // Call Unref() on the underlying object pointer.
    ~SharedPtr() {
        SafeUnref(ptr_);
    }

    SharedPtr<T>& operator=(std::nullptr_t) {
        this->reset();
        return *this;
    }

    // Shares the underlying object referenced by the argument by calling Ref() on it. If this
    // SharedPtr previously had a reference to an object  (i.e. not null) it will call Unref()
    // on that object.
    SharedPtr<T>& operator=(const SharedPtr<T>& that) {
        if (this != &that) {
            this->reset(SafeRef(that.get()));
        }
        return *this;
    }

    // Move the underlying object from the argument to the SharedPtr. If the SharedPtr previously held
    // a reference to another object, Unref() will be called on that object. No  call to Ref()
    // will be made.
    SharedPtr<T>& operator=(SharedPtr<T>&& that) noexcept {
        this->reset(that.release());
        return *this;
    }

    T& operator*() const {
        return *this->get();
    }

    explicit operator bool() const {
        return this->get() != nullptr;
    }

    [[nodiscard]] T* get() const { return ptr_; }

    T* operator->() const { return ptr_; }

    // Adopt the new bare pointer, and call Unref() on any previously held object (if not null).
    void reset(T* ptr = nullptr) {
        T* old_ptr = ptr_;
        ptr_ = ptr;
        SafeUnref(old_ptr);
    }

    // return the bare pointer, and set the internal object pointer to nullptr.
    // The Caller must assume ownership of the object, and manage its reference count directly.
    // No call to Unref() will be made.
    T* release() {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    void swap(SharedPtr<T>& that) {
        std::swap(ptr_, that.ptr_);
    }

private:
    T* ptr_;
};

template<typename T, typename... Args>
SharedPtr<T> MakeSharedPtr(Args&& ... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

namespace internal {
    class Student : public RefCnt {
    public:
        Student() {
            std::cout << __FUNCTION__ << std::endl;
            count_ = 25;
            score_ = static_cast<float*>(malloc(count_ * sizeof(float)));
        }

        ~Student() override {
            std::cout << __FUNCTION__ << std::endl;
            if (score_) {
                free(score_);
                score_ = nullptr;
            }
        }

        [[nodiscard]] int count() const {
            return count_;
        }

    private:
        float* score_ = nullptr;
        int count_ = 0;
        std::string name_;
    };
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    {
        std::cout << "## shared_ptr结构体" << std::endl;
        std::shared_ptr<internal::Student> student1 =
                std::make_shared<internal::Student>();
        std::cout << "student1 count " << student1->count() << std::endl;
        const std::shared_ptr<internal::Student>& student2 = student1;
        std::cout << "student2 count " << student2->count() << std::endl;
    }

    {
        std::cout << "## unique_ptr结构体" << std::endl;
        std::unique_ptr<internal::Student> student3 =
                std::make_unique<internal::Student>();
        std::cout << "student3 count " << student3->count() << std::endl;
        std::unique_ptr<internal::Student> student4 = std::move(student3);
        if (student3 == nullptr) {
            std::cout << "After move() function, student3 is nullptr" << std::endl;
        }
        // 此时如果替换成为student3->count()程序会发生崩溃
        std::cout << "student4 count " << student4->count() << std::endl;
    }

    {
        std::cout << "## 自定义引用计数" << std::endl;
        SharedPtr<internal::Student> student5 = MakeSharedPtr<internal::Student>();
        std::cout << "student5 unique " << (student5->Unique() ? "true" : "false") << std::endl;
        SharedPtr<internal::Student> student6 = student5;
        std::cout << "student6 unique " << (student6->Unique() ? "true" : "false") << std::endl;
        student5.reset();
        std::cout << "student6 unique " << (student6->Unique() ? "true" : "false") << std::endl;
        student6.swap(student5);
        std::cout << "student5 unique " << (student5->Unique() ? "true" : "false") << std::endl;
    }
    return 0;
}
