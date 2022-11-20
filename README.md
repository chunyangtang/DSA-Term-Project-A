# DSA-Term-Project-A

## 环境简介

- g++：

  Apple clang version 14.0.0 (clang-1400.0.29.202)
  Target: arm64-apple-darwin22.1.0
  Thread model: posix
- python

  Python 3.10.8
- operating system

  macOS 13.0.1

## 文件结构说明

根目录下的 data_basic1, data_basic2, data_test 为原始提供数据，未做处理。

test_folder_base, test_folder_advanced 分别代表基础任务，感知哈希任务在实际操作中处理的数据与生成的结果。

io.cpp, utils.py为提供的代码未做修改。

main.cpp 为基础实现的代码。

main_dct_pc.cpp, dct_pc.py 为实现感知哈希的最终代码。

main_dct.cpp, my_dct.py 为在c中调用python的pHash代码，但不易配置环境与运行。

main_dct_c.cpp 为完全使用c++的pHash代码，运行速度极为缓慢。

makefile 为编译用到的指令。
