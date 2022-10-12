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