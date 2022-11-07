import argparse
import os
import pathlib
import sys

import numpy as np
from matplotlib import pyplot as plt

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='PlotWaveform')
    parser.add_argument('datapath', type=pathlib.Path)
    parser.add_argument('-s', '--sample_format',
                        help='audio sample format, eg: s16le')
    args = parser.parse_args()

    if args.sample_format == 'f32le':
        buffer_size = int(os.stat(args.datapath).st_size / 4)
        x = np.linspace(0, buffer_size, buffer_size)
        y = np.fromfile(args.datapath, dtype=np.float32,
                        count=-1, offset=0)
    else:
        buffer_size = int(os.stat(args.datapath).st_size / 2)
        x = np.linspace(0, buffer_size, buffer_size)
        y = np.fromfile(args.datapath, dtype=np.int16,
                        count=-1, offset=0)

    fig, ax = plt.subplots()
    ax.plot(x, y)
    plt.show()
