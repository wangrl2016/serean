//
// Created by wangrl2016 on 2022/10/29.
//

#include <cstdlib>
#include <cstdio>
#include <cstddef>

// 1. 单极指针和二级指针的介绍
// 2. 二级指针在实战中的使用

// 无法实现两个参数的交换
void ErrorSwap(int a, int b) {
    int temp = b;
    b = a;
    a = temp;
}

// 使用指针实现
void PointerSwap(int* a, int* b) {
    int temp = *b;
    *b = *a;
    *a = temp;
}

// 使用引用实现
void ReferenceSwap(int& a, int& b) {
    int temp = b;
    b = a;
    a = temp;
}

// 二级指针初始化错误实现
void ErrorInitPointer(void* ptr, size_t num) {
    ptr = malloc(num);
}

// 正确实现
void InitPointer(void** ptr, size_t num) {
    (*ptr) = malloc(num);
}

int main(int argc, char* argv[]) {
    int var1 = 11;
    int var2 = 22;
    printf("var1 %d, var2 %d\n", var1, var2);
    ErrorSwap(var1, var2);
    printf("var1 %d, var2 %d\n", var1, var2);
    int* ptr1 = &var1;
    int* ptr2 = &var2;
    PointerSwap(ptr1, ptr2);
    printf("var1 %d, var2 %d\n", var1, var2);
    int& ref1 = var1;
    int& ref2 = var2;
    ReferenceSwap(ref1, ref2);
    printf("var1 %d, var2 %d\n", var1, var2);

    int* buffer = nullptr;
    ErrorInitPointer(buffer, 10);
    if (buffer == nullptr)
        printf("buffer is nullptr\n");

    InitPointer(reinterpret_cast<void**>(&buffer), 10);
    if (buffer)
        printf("buffer inited\n");
    return EXIT_SUCCESS;
}
