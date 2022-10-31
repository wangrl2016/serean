import gzip
import os.path
import struct

import numpy as np
import requests
from matplotlib import pyplot

mnist_database = [
    'http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz',
    'http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz',
    'http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz',
    'http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz'
]

temp_dir = 'temp'

if __name__ == '__main__':
    image_arr = []
    label_arr = []

    if not os.path.exists(temp_dir):
        os.mkdir(temp_dir)

    for url in mnist_database:
        filename = url.split('/')[-1]
        uncompress_name = filename.replace('.gz', '')
        if not os.path.exists(os.path.join(temp_dir, filename)):
            print('Downloading ' + filename)
            response = requests.get(url)
            open(os.path.join(temp_dir, filename), 'wb').write(response.content)

            content = gzip.GzipFile(os.path.join(temp_dir, filename))
            open(os.path.join(temp_dir, uncompress_name), 'wb+').write(content.read())
            content.close()

        if uncompress_name.__contains__('image'):
            with open(os.path.join(temp_dir, uncompress_name), 'rb') as image:
                # >IIII big-ending, four int
                magic, num, rows, cols = struct.unpack('>IIII', image.read(16))
                print('magic ' + str(magic) + ', num ' + str(num)
                      + ', rows ' + str(rows) + ', cols ' + str(cols))
                image_arr.append(np.fromfile(image, dtype=np.uint8).reshape(num, 28 * 28))

        if uncompress_name.__contains__('label'):
            with open(os.path.join(temp_dir, uncompress_name), 'rb') as label:
                magic, num = struct.unpack('>II', label.read(8))
                print('magic ' + str(magic) + ', num ' + str(num))
                label_arr.append(np.fromfile(label, dtype=np.uint8))

    fig, ax = pyplot.subplots(nrows=8, ncols=10, sharex='all', sharey='all')
    ax = ax.flatten()
    for i in range(80):
        img = image_arr[0][i].reshape(28, 28)
        ax[i].imshow(img, cmap='Greys', interpolation='nearest')

    ax[0].set_xticks([])
    ax[0].set_yticks([])
    pyplot.tight_layout()
    pyplot.show()
