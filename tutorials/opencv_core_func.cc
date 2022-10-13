//
// Created by wangrl2016 on 2022/10/12.
//

#include <iostream>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/highgui.hpp>

using namespace cv;

int main(int argc, char* argv[]) {
    // 倒序查看示例

    // 1. 数字设备获取图像
    // 2. Mat结构体的两大部分：
    // 矩阵头包含矩阵的大小，指向像素值的指针
    // 实体数据
    // 3. 自动管理内存
    // 使用clone()和copyTo()进行数据复制
    // 4. 储存的数据有RGB, HLS, YCrCb
    // 数据格式：CV_8UC3, CV_64F
    // 5. 介绍Mat提供的函数
    {
        std::cout << "This program shows how to create matrices(cv::Mat)"
                     "in OpenCV and its serial out capabilities" << std::endl;

        // Create by using the constructor.
        Mat M(2, 2, CV_8UC3, Scalar(0, 0, 255));
        std::cout << "M = " << std::endl << " " << M << std::endl << std::endl;

        // Create by using the create function.
        M.create(4, 4, CV_8UC(3));
        std::cout << "M = " << std::endl << " " << M << std::endl << std::endl;

        // Create multidimensional matrices
        int sz[3] = {2, 2, 2};
        Mat L(3, sz, CV_8UC(1), Scalar::all(0));

        // Fill a matrix with random values
        Mat R = Mat(3, 2, CV_8UC3);
        randu(R, Scalar::all(0), Scalar::all(255));
        std::cout << "R (default) = " << std::endl << R << std::endl << std::endl;

        // Create using MATLAB style eye, ones or zero matrix
        Mat E = Mat::eye(4, 4, CV_64F);
        std::cout << "E = " << std::endl << " " << E << std::endl << std::endl;
        Mat O = Mat::ones(2, 2, CV_32F);
        std::cout << "O = " << std::endl << " " << O << std::endl << std::endl;
        Mat Z = Mat::zeros(3, 3, CV_8UC1);
        std::cout << "Z = " << std::endl << " " << Z << std::endl << std::endl;

        // create a 3x3 double-precision identity matrix
        Mat C = (Mat_<double>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
        std::cout << "C = " << std::endl << " " << C << std::endl << std::endl;

        Mat RowClone = C.row(1).clone();
        std::cout << "RowClone = " << std::endl << " "
                  << RowClone << std::endl << std::endl;

        Point2f P(5, 1);
        std::cout << "Point (2D) = " << P << std::endl << std::endl;

        Point3f P3f(2, 6, 7);
        std::cout << "Point (3D) = " << P3f << std::endl << std::endl;

        std::vector<float> v;
        v.push_back((float) CV_PI);
        v.push_back(2);
        v.push_back(3.01f);
        std::cout << "Vector of floats via Mat = " << Mat(v)
                  << std::endl << std::endl;

        std::vector<Point2f> vPoints(20);
        for (size_t i = 0; i < vPoints.size(); ++i)
            vPoints[i] = Point2f((float) (i * 5), (float) (i % 7));
        std::cout << "A vector of 2D Points = " << vPoints << std::endl << std::endl;
    }

    // 1. 使用Conan安装OpenCV库
    // 2. 通过imread()函数导入图片
    // 3. 通过imshow()函数显示图片
    // 4. 通过imwrite()函数保存图片
    {
        if (argc != 3) {
            std::cout << "Need image path" << std::endl;
            return EXIT_FAILURE;
        }

        std::string image_path =
                cv::samples::findFile(argv[1]);
        Mat img = imread(image_path, IMREAD_COLOR);
        if (img.empty()) {
            std::cout << "Could not read the image: " << image_path << std::endl;
            return EXIT_FAILURE;
        }

        imshow("Display window", img);
        int k = waitKey(0); // wait for a keystroke in the window
        if (k == 's') {
            imwrite(argv[2], img);
        }

        return 0;
    }


}