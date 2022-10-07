//
// Created by wangrl2016 on 2022/9/22.
//

#include <cstdlib>
#include <iostream>

// 编写节拍器7个章节:
// 1. 导读
// 2. C语言内存管理
// 3. C++智能指针
// 4. 声音的表示
// 5. 示波器
// 6. 编写macOS播放器
// 7. 跨平台拓展

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // 1. 简单介绍开发环境
    // 电脑型号macOS
    // 编辑器CLion
    // 编译工具链Apple clang
    // 源码地址https://github.com/wangrl2016/serean

    // 2. man malloc查看内存管理函数族
    // calloc, free, malloc, realloc, reallocf, valloc, aligned_alloc – memory allocation

    // 3. malloc/free函数介绍
    // void* malloc(size_t size);
    // The malloc() function allocates size bytes of memory and returns a pointer
    // to the allocated memory.
    {
        int* pi1;
        pi1 = static_cast<int*>(malloc(100));
        if (pi1 == nullptr) {
            std::cout << "Out of memory" << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << "## malloc函数" << std::endl;
        std::cout << "地址: " << &(pi1[0]) << ", 数值: " << pi1[0] << std::endl;
        std::cout << "地址: " << &(pi1[1]) << ", 数值: " << pi1[1] << std::endl;

        for (int i = 0; i < 25; i++)
            pi1[i] = i;
        std::cout << "第10个元素数值: " << pi1[9] << std::endl;
        free(pi1);
    }

    // 4. calloc函数介绍
    // void* calloc(size_t count, size_t size);
    // The calloc() function contiguously allocates enough space for count objects
    // that are size bytes of memory each and returns a pointer to the allocated memory.
    // The allocated memory is filled with bytes of value zero.
    {
        int* pi2;
        pi2 = static_cast<int*>(calloc(25, sizeof(int)));
        if (pi2 == nullptr) {
            std::cout << "Out of memory" << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << "## calloc函数" << std::endl;
        std::cout << "地址: " << &(pi2[0]) << ", 数值: " << pi2[0] << std::endl;
        std::cout << "地址: " << &(pi2[1]) << ", 数值: " << pi2[1] << std::endl;

        for (int i = 0; i < 25; i++)
            pi2[i] = i;
        std::cout << "第10个元素数值: " << pi2[9] << std::endl;
        free(pi2);
    }

    // 5. realloc函数介绍
    // void* realloc(void *ptr, size_t size);
    // The realloc() function tries to change the size of the allocation pointed to
    // by ptr to size, and returns ptr. If there is not enough room to enlarge the
    // memory allocation pointed to by ptr, realloc() creates a new allocation, copies
    // as much of the old data pointed to by ptr as will fit to the new allocation,
    // frees the old allocation, and returns a pointer to the allocated memory.
    {
        std::cout << "## realloc函数" << std::endl;
        int* pi3;
        pi3 = static_cast<int*>(malloc(10 * sizeof(int)));
        if (pi3 == nullptr) {
            std::cout << "Out of memory" << std::endl;
            return EXIT_FAILURE;
        }
        for (int i = 0; i < 10; i++)
            pi3[i] = i;

        pi3 = static_cast<int*>(realloc(pi3, 25 * sizeof(int)));
        if (pi3 == nullptr) {
            std::cout << "Out of memory" << std::endl;
            return EXIT_FAILURE;
        }
        std::cout << "第5个元素数值: " << pi3[4] << std::endl;
        // realloc函数并不负责将额外分配的内存清零
        std::cout << "第20个元素数值: " << pi3[19] << std::endl;

        for (int i = 10; i < 25; i++)
            pi3[i] = 0;
        std::cout << "第20个元素数值: " << pi3[19] << std::endl;
        free(pi3);
    }

    // 6. 内存操作函数memset/memcpy介绍
    {
        std::cout << "## memset函数" << std::endl;
        int* pi4;
        pi4 = static_cast<int*>(malloc(10 * sizeof(int)));
        if (pi4 == nullptr) {
            std::cout << "Out of memory" << std::endl;
            return EXIT_FAILURE;
        }
        for (int i = 0; i < 10; i++)
            pi4[i] = i;
        std::cout << "第2个元素值: " << pi4[1] << std::endl;
        memset(pi4, 0, 10 * sizeof(int));
        std::cout << "第2个元素值: " << pi4[1] << std::endl;
        std::cout << "第5个元素值: " << pi4[4] << std::endl;
        std::cout << "第6个元素值: " << pi4[5] << std::endl;

        std::cout << "## memcpy函数" << std::endl;
        int* pi5;
        pi5 = static_cast<int*>(malloc(10 * sizeof(int)));
        if (pi5 == nullptr) {
            std::cout << "Out of memory" << std::endl;
            return EXIT_FAILURE;
        }
        for (int i = 0; i < 10; i++)
            pi5[i] = i;
        // 复制pi5后5个元素到pi4的后5个内存中
        memcpy(&(pi4[5]), &(pi5[5]), 5 * sizeof(int));
        std::cout << "第5个元素值: " << pi4[4] << std::endl;
        std::cout << "第6个元素值: " << pi4[5] << std::endl;
        std::cout << "第10个元素值: " << pi4[9] << std::endl;
        free(pi4);
        free(pi5);
    }

    return 0;
}
