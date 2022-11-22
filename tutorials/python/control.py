

if __name__ == '__main__':
    a = 0
    b = 10
    c = 10
    if a < b == c:
        print('a 是否小于 b，且 b 是否等于 c')
    else:
        print('a 是否小于 b 和 c 的比较值')

    if (a := 3) >= 0:
        print('a = ' + str(a))

