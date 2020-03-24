#include <jni.h>
#include <android/log.h>

#define ARRAY_LENGTH(array) sizeof(array) / sizeof(array[0])

#define TAG "libyuv_utils"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,   TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,    TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARNING, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,   TAG, __VA_ARGS__)