//
// Created by wangrl2016 on 2022/9/22.
//

#include <iostream>

namespace internal {
    class Student {
    public:
        Student() {
            std::cout << __FUNCTION__ << std::endl;
            count_ = 25;
            score_ = static_cast<float*>(malloc(count_ * sizeof(float)));
        }

        ~Student() {
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
    };

    // RefCnt is the base class for objects that may be shared by multiple
    // objects. When an existing owner wants to share a reference, it calls ref().
    // When an owner wants to release its reference, it calls unref(). When the
    // shared object's reference count goes to zero as the result of an unref()
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

        }

    private:
        mutable std::atomic<int32_t> ref_cnt_;



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
    }
    return 0;
}
