//
// Created by wangrl2016 on 2022/10/12.
//

#include <iostream>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/highgui.hpp>

using namespace cv;

Mat& ScanImageAndReduceC(Mat& I, const uchar* table) {
    // accept only char type matrices
    CV_Assert(I.depth() == CV_8U);

    int channels = I.channels();
    int n_rows = I.rows;            // 列
    int n_cols = I.cols * channels; // 排
    if (I.isContinuous()) {
        n_cols *= n_rows;
        n_rows = 1;
    }

    uchar* p;
    for (int i = 0; i < n_rows; i++) {
        p = I.ptr<uchar>(i);
        for (int j = 0; j < n_cols; j++) {
            p[j] = table[p[j]];
        }
    }
    return I;
}

Mat& ScanImageAndReduceIterator(Mat& I, const uchar* const table) {
    CV_Assert(I.depth() == CV_8U);

    const int channels = I.channels();
    switch (channels) {
        case 1: {
            MatIterator_<uchar> it, end;
            for (it = I.begin<uchar>(), end = I.end<uchar>();
                 it != end; it++)
                *it = table[*it];
            break;
        }
        case 3: {
            MatIterator_<Vec3b> it, end;
            for (it = I.begin<Vec3b>(), end = I.end<Vec3b>();
                 it != end; it++) {
                (*it)[0] = table[(*it)(0)];
                (*it)[1] = table[(*it)[1]];
                (*it)[2] = table[(*it)[2]];
            }
        }
        default:
            break;
    }
    return I;
}

Mat& ScanImageAndReduceRandomAccess(Mat& I, const uchar* const table) {
    CV_Assert(I.depth() == CV_8U);

    const int channels = I.channels();
    switch (channels) {
        case 1: {
            for (int i = 0; i < I.rows; i++)
                for (int j = 0; j < I.cols; j++)
                    I.at<uchar>(i, j) =
                            table[I.at<uchar>(i, j)];
            break;
        }
        case 3: {
            Mat_<Vec3b> II = I;
            for (int i = 0; i < I.rows; i++)
                for (int j = 0; j < I.cols; j++) {
                    II(i, j)[0] = table[II(i, j)[0]];
                    II(i, j)[1] = table[II(i, j)[1]];
                    II(i, j)[2] = table[II(i, j)[2]];
                }
            I = II;
            break;
        }
        default:
            break;
    }
    return I;
}

int main(int argc, char* argv[]) {
    // 倒序查看示例

    // 矩阵遍历的四种方法
    // 1. The efficient way
    // 2. The iterator (safe method)
    // 3. On-the-fly address calculation with reference returning
    // 4. cv::LUT() function
    //
    // 算法：I(new) = (I(old) / 10) * 10
    // 例如 (128 / 10) * 10 = 12 * 10 = 120
    //
    // 通过查表的方式减少算法的计算
    // table[128] = 120;
    // table[127] = 120;
    // ...
    {
        std::cout << "This program shows how to scan image objects\n"
                     "in OpenCV (cv::Mat). As use case we take an\n"
                     "input image and divide the native color\n"
                     "palette (255) with the input. Shows C operator[]\n"
                     "method, iterators and at function for on-the-fly\n"
                     "item address calculation." << std::endl;

        if (argc < 3) {
            std::cout << "Not enough parameters" << std::endl;
            return EXIT_FAILURE;
        }

        Mat I, J;
        if (argc == 4 && !strcmp(argv[3], "G"))
            I = imread(argv[1], IMREAD_GRAYSCALE);
        else
            I = imread(argv[1], IMREAD_COLOR);

        if (I.empty()) {
            std::cout << "The image " << argv[1]
                      << " could not be loaded." << std::endl;
            return EXIT_FAILURE;
        }

        // Convert out input string to number - C++ style.
        int divide_with = 0;
        std::stringstream s;
        s << argv[2];
        s >> divide_with;
        if (!s || !divide_with) {
            std::cout << "Invalid number entered for dividing." << std::endl;
            return EXIT_FAILURE;
        }

        uchar table[256];
        for (int i = 0; i < 256; i++)
            table[i] = (uchar) (divide_with * (i / divide_with));

        const int times = 20;
        double t;

        t = (double) getTickCount();
        for (int i = 0; i < times; i++) {
            cv::Mat clone_i = I.clone();
            J = ScanImageAndReduceC(clone_i, table);
        }
        if (J.empty()) {
            std::cout << "Mat is empty" << std::endl;
            return EXIT_FAILURE;
        }

        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        t /= times;

        std::cout << "Time of reducing with the C operator [] (averaged for "
                  << times << " runs): " << t << " milliseconds." << std::endl;

        t = (double) getTickCount();
        for (int i = 0; i < times; ++i) {
            cv::Mat clone_i = I.clone();
            J = ScanImageAndReduceIterator(clone_i, table);
        }

        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        t /= times;

        std::cout << "Time of reducing with the iterator (averaged for "
                  << times << " runs): " << t << " milliseconds." << std::endl;

        t = (double) getTickCount();

        for (int i = 0; i < times; ++i) {
            cv::Mat clone_i = I.clone();
            ScanImageAndReduceRandomAccess(clone_i, table);
        }

        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        t /= times;

        std::cout << "Time of reducing with the on-the-fly address generation - at function (averaged for "
                  << times << " runs): " << t << " milliseconds." << std::endl;

        Mat look_up_table(1, 256, CV_8U);
        uchar* p = look_up_table.ptr();
        for (int i = 0; i < 256; ++i)
            p[i] = table[i];

        t = (double) getTickCount();
        for (int i = 0; i < times; i++)
            LUT(I, look_up_table, J);

        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        t /= times;

        std::cout << "Time of reducing with the LUT function (averaged for "
                  << times << " runs): " << t << " milliseconds." << std::endl;
    }

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
    }
    return 0;
}