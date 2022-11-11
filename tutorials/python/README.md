## 从零开始学Python

官方网址`https://docs.python.org/zh-cn/3/tutorial/`

### 第01期：《从零开始学Python》开篇介绍以及Python安装

* 针对的对象

想要学习Python的小伙伴

* 安装

1. macOS

系统已经自带，或者`brew install python3`

2. Linux(Ubuntu)

系统自带，或者`apt install python3`

3. Windows

应用商店安装`Python`或者下载`https://www.python.org/downloads/windows/`

应用商店安装`Windows Terminal`

### 第02期：Python解释器输出"Hello World"语句

* Python优势

平常使用电脑可以批量处理任务

软件开发者可以自动软件开发流程

* `-c command`可以进行指令输出

* 理解`···`和`>>>`的区别

`...`指令没有输入完成可以继续输入

`>>>`提示输入下一条指令

### 第03期：Python计算器：数字、字符串

* 数字

加减乘除计算

* 字符串

字符串的表示

### 第04期：Python基础数据结构：列表（list）及内部实现

* 列表介绍

`[1, 2, 3, 4, 5]`

* 列表内部实现

网址：https://www.laurentluce.com/posts/python-list-implementation

```
typedef struct {
    PyObject_VAR_HEAD
    PyObject **ob_item;
    Py_ssize_t allocated;
} PyListObject;
```

### 第05期：初识Python算法：斐波那契数

* 数列是什么？

* 如何编写？

### 第06期：Python流程控制（顺序、条件、循环）语句详解

### 第07期：Python函数定义（函数名、参数、返回值）



### 第08期：递归(Recursion)思想分析汉诺塔问题

* 迭代

```
>>> def factorial(n):
...     answer = 1
...     for i in range(1, n + 1):
...         answer = answer * i
...     return answer
... 
>>> print(factorial(4))
```

* 递归

```
>>> def factorial(n):
...     if n == 0:
...         return 1
...     else:
...         return n * factorial(n - 1)
... 
>>> print(factorial(4))
```

```
def hanoi(n, a, b, c):
	if n == 1:
		print(a, '-->', c)
	else:
		hanoi(n - 1, a, c, b)
		hanoi(1    , a, b, c)
		hanoi(n - 1, b, a, c)
# 调用
if __name__ == '__main__':
	hanoi(5, 'A', 'B', 'C')
```

`http://simonsays-tw.com/web/Recursion/Iteration&Recursion.html`

`http://simonsays-tw.com/web/Recursion/TowerOfHanoi.html`

`https://zh.wikipedia.org/zh-cn/汉诺塔`

`https://zh.wikipedia.org/zh-cn/递归`

### 第09期：函数参数(*arguments, **keywords)详解

### 第10期：Lambda表达式和Python编码规范



