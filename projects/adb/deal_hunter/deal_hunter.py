import argparse
import subprocess
import time
from random import randrange


def swipe(pid, w1, h1, w2, h2, internal=200, gap=3):
    """
    滑动屏幕

    :param pid: 手机标识符
    :param w1: 第一个点水平方向像素坐标
    :param h1: 第一个点竖直防线像素坐标
    :param w2: 第二个点水平方向像素坐标
    :param h2: 第二个点竖直方向像素坐标
    :param internal: 两个点之间的滑动间隔时间
    :param gap: 时间间隔
    """
    subprocess.run(['adb', '-s', pid, 'shell', 'input', 'swipe',
                    str(int(w1)), str(int(h1)),
                    str(int(w2)), str(int(h2)), str(internal)])
    time.sleep(gap)


def swipe_down_to_up(pid, w, h, gap=3, internal=200):
    """
    从下往上滑动屏幕
    """
    w = w + randrange(-25, 25)
    h = h + randrange(-50, 50)
    internal = internal + randrange(-30, 30)
    subprocess.run(['adb', '-s', pid, 'shell', 'input', 'swipe',
                    str(int(w)), str(int(h * 3 / 4)),
                    str(int(w)), str(int(h * 1 / 4)),
                    str(internal)])
    time.sleep(gap)


def get_size(pid):
    """
    获取手机的像素点大小
    """
    p = subprocess.run(['adb', '-s', pid, 'shell', 'wm', 'size'],
                       check=True, stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT, universal_newlines=True)
    size_str = p.stdout.strip('\n')
    for s in size_str.split(' '):
        if s.rfind('x') > 0:
            return int(s.split('x')[0]), int(s.split('x')[1])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog='PROG', conflict_handler='resolve')
    parser.add_argument('-s', '--serial', help='phone serial number')

    while True:
        swipe_down_to_up('9598552235004UD', 1080 / 2, 2040, randrange(5, 16))
