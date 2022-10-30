import argparse
import os
import pathlib

import numpy as np
from matplotlib import pyplot as plt

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='PlotWaveform')
    parser.add_argument('datapath', type=pathlib.Path)
    args = parser.parse_args()

    buffer_size = int(os.stat(args.datapath).st_size / 2)
    x = np.linspace(0, buffer_size, buffer_size)
    y = np.fromfile(args.datapath, dtype=np.int16,
                    count=-1, offset=0)

    fig, ax = plt.subplots()
    ax.plot(x, y)
    plt.show()
