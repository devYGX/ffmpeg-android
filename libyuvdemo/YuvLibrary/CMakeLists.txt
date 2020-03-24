CMAKE_MINIMUM_REQUIRED(VERSION 3.4.1)

# 指定头文件路径
INCLUDE_DIRECTORIES(src/main/cpp/libyuv/include src/main/cpp/include)

# 添加编译的子模块, 这个路径下将也有一个CMakeLists.txt编译文件
ADD_SUBDIRECTORY(src/main/cpp/libyuv)

# 查找目录下的所有c源文件并赋值给YUV_SOURCE中
AUX_SOURCE_DIRECTORY(src/main/cpp SOURCE)
AUX_SOURCE_DIRECTORY(src/main/cpp/libyuv/source LIBYUV_SOURCE)

# 添加编译的库, 库名: yuvlib_utils, SHARED表示是动态库, ${SOURCE}是编译源码的路径
ADD_LIBRARY(libyuv_utils SHARED ${SOURCE} ${LIBYUV_SOURCE})

# 查找库
FIND_LIBRARY(log-lib log)

# 关联库
TARGET_LINK_LIBRARIES(libyuv_utils ${log-lib})