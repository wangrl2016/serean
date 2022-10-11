//
// Created by wangrl2016 on 2022/10/10.
//

#include <vector>
#include <fstream>
#include <tuple>

// 数字图像基础
//
// 1. 色彩原理和生成PPM彩色图像
//
// Introduction to OpenCV
// 2. OpenCV库的安装和介绍
//
// The Core Functionality (core module)
// 3. 讲解Mat核心数据结构
// 4. 几种快速遍历矩阵的方法
// 5. 锐化
// 6. 图像的操作
// 7. 两张图片叠加
// 8. 改变图片的对比度和亮度
// 9. 离散傅立叶变换
//
// 1. 三原色原理
// 三原色光模式（RGB color model），又称RGB颜色模型或红绿蓝颜色模型，
// 是一种加色模型，将红（Red）、绿（Green）、蓝（Blue）三原色的色光以不同的比例相加，
// 以合成产生各种色彩光。
// 每象素24位（bits per pixel，bpp）编码的RGB值：使用三个8位无符号整数（0到255）
// 表示红色、绿色和蓝色的强度。这是当前主流的标准表示方法，用于真彩色和JPEG或者TIFF
// 等图像文件格式里的通用颜色交换。
// (0, 0, 0)是黑色
// (255, 255, 255)是白色
// (255, 0, 0)是红色
// (0, 255, 0)是绿色
// (0, 0, 255)是蓝色
// (255, 255, 0)是黄色
// (0, 255, 255)是青色
// (255, 0, 255)是品红
//
// 2. PPM图像格式
// P6
// width height
// 255
// rgb rgb rgb ...


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    const int width = 1920;
    const int height = 1080;

    std::vector<std::tuple<float, float, float>> frame_buffer(width * height);

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            frame_buffer[i + j * width] = {float(j) / float(height), float(i) / float(width), 0};
        }
    }

    std::ofstream ofs;  // save the frame buffer to file
    ofs.open("./simple_1920x1080.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            auto& point = frame_buffer[i + j * width];
            ofs << (char) (255 * std::max(0.0f, std::min(1.0f, std::get<0>(point))));
            ofs << (char) (255 * std::max(0.0f, std::min(1.0f, std::get<1>(point))));
            ofs << (char) (255 * std::max(0.0f, std::min(1.0f, std::get<2>(point))));
        }
    }
    ofs.close();

    return 0;
}