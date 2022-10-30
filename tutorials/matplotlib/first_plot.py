import matplotlib.pyplot as plt
import numpy as np

# pip3 install matplotlib

if __name__ == '__main__':
    x = np.linspace(0, 2 * np.pi, 200)
    y = np.sin(x)

    # 1. figure
    # fig = plt.figure()  # an empty figure with no Axes
    fig, ax = plt.subplots()  # a figure with a single Axes
    # fig, axs = plt.subplots(2, 2)

    # 2. Axes
    ax.plot(x, y)  # 绘制坐标点

    # 3. Title
    ax.set_title('Sin function')

    # 4. xlabel
    ax.set_xlabel('XXX')

    # 5. ylabel
    ax.set_ylabel('YYY')

    plt.show()
