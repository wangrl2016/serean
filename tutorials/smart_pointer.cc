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
        std::cout << "student4 count " << student4->count() << std::endl;
    }
    return 0;
}
