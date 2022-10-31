import tensorflow as tf

# 要解决的问题是，将手写数字的灰度图像(28像素×28像素)划分到10个类别中(0~9)。
# 我们将使用MNIST数据集，它是机器学习领域的一个经典数据集，其历史几乎和这个领域
# 一样长，而且已被人们深入研究。这个数据集包含60000张训练图像和10000张测试图
# 像，由美国国家标准与技术研究院(National Institute of Standards and Technology，
# 即MNIST中的NIST)在20世纪80年代收集得到。

if __name__ == '__main__':

    # http://yann.lecun.com/exdb/mnist/
    mnist = tf.keras.datasets.mnist

    (x_train, y_train), (x_test, y_test) = mnist.load_data()
    x_train, x_test = x_train / 255.0, x_test / 255.0
