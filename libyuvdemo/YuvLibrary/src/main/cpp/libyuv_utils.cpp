#include <cstring>
#include "libyuv.h"
#include "base.h"

#define CLASS_NAME "org/libyuv/utils/LibYuvUtils"

#define CHECK_SIZE(w, h, pixel, target) (w * h * pixel == target)


#define PIXEL_NV21 (float)3 / 2
#define PIXEL_I420 (float)3 / 2
#define PIXEL_RGB24 3

#define INVALID_ARGS -1
#define DISMATCH_SIZE -2

int nv21ToI420(uint8_t *nv21, uint8_t *i420, int width, int height) {

    return libyuv::NV21ToI420(nv21, width,
                              nv21 + width * height, width,
                              i420, width,
                              i420 + width * height, width >> 1,
                              i420 + width * height + (width >> 1) * (height >> 1), width >> 1,
                              width, height);
}

int rotateNV21ToNv21(uint8_t *nv21, uint8_t *dst_nv21, int width, int height, int size,
                     libyuv::RotationMode mode) {
    uint8_t *i420_src = (uint8_t *) malloc(size);
    uint8_t *i420_rotate = (uint8_t *) malloc(size);

    libyuv::NV21ToI420(nv21, width,
                       nv21 + width * height, width,
                       i420_src, width,
                       i420_src + width * height, width >> 1,
                       i420_src + width * height + (width >> 1) * (height >> 1), width >> 1,
                       width, height);
    int new_width = width;
    int new_height = height;
    if (mode == libyuv::kRotate90 || mode == libyuv::kRotate270) {
        new_width = height;
        new_height = width;
    }

    libyuv::I420Rotate(i420_src, width,
                       i420_src + width * height, width >> 1,
                       i420_src + width * height + (width >> 1) * (height >> 1), width >> 1,
                       i420_rotate, new_width,
                       i420_rotate + new_width * new_height, new_width >> 1,
                       i420_rotate + new_width * new_height + (new_width >> 1) * (new_height >> 1),
                       new_width >> 1,
                       width, height, mode
    );

    libyuv::I420ToNV21(i420_rotate, new_width,
                       i420_rotate + new_width * new_height, new_width >> 1,
                       i420_rotate + new_width * new_height + (new_width >> 1) * (new_height >> 1),
                       new_width >> 1,
                       dst_nv21, new_width,
                       dst_nv21 + (new_width * new_height), new_width,
                       new_width, new_height
    );

    free(i420_src);
    free(i420_rotate);
    return 0;
}


int rotateNV21ToI420(uint8_t *nv21, uint8_t *dst_i420, int width, int height, int size,
                     libyuv::RotationMode mode) {
    uint8_t *i420_src = (uint8_t *) malloc(size);

    libyuv::NV21ToI420(nv21, width, nv21 + width * height, width,
                       i420_src, width,
                       i420_src + width * height, width >> 1,
                       i420_src + width * height + (width >> 1) * (height >> 1), width >> 1, width,
                       height);
    int new_width = width;
    int new_height = height;
    if (mode == libyuv::kRotate90 || mode == libyuv::kRotate270) {
        new_width = height;
        new_height = width;
    }
    libyuv::I420Rotate(i420_src, width,
                       i420_src + width * height, width >> 1,
                       i420_src + width * height + (width >> 1) * (height >> 1), width >> 1,
                       dst_i420, new_width,
                       dst_i420 + width * height, new_width >> 1,
                       dst_i420 + width * height + (new_width >> 1) * (new_height >> 1),
                       new_width >> 1,
                       width, height, mode
    );

    free(i420_src);
    return 0;
}

static jint
nativeRotateNv21ToNv21(JNIEnv *env, jobject jclas, jbyteArray nv21Src, jbyteArray nv21Output,
                       jint width, jint height,
                       jint rotation) {
    if (nv21Src == NULL || nv21Output == NULL) {
        return INVALID_ARGS;
    }

    jsize srcSize = env->GetArrayLength(nv21Src);
    if (!CHECK_SIZE(width, height, PIXEL_NV21, srcSize)) {
        return DISMATCH_SIZE;
    }
    jsize dstSize = env->GetArrayLength(nv21Output);
    if (!CHECK_SIZE(width, height, PIXEL_NV21, dstSize)) {
        return DISMATCH_SIZE;
    }

    jbyte *src = env->GetByteArrayElements(nv21Src, NULL);
    jbyte *dst = env->GetByteArrayElements(nv21Output, NULL);


    switch (rotation) {
        case 0: {
            memcpy(dst, src, srcSize);
        }
            break;
        case 90: {
            rotateNV21ToNv21(
                    (uint8_t *) src, (uint8_t *) dst, width, height, srcSize, libyuv::kRotate90);
        }
            break;
        case 180: {
            rotateNV21ToNv21(
                    (uint8_t *) src, (uint8_t *) dst, width, height, srcSize, libyuv::kRotate180);
        }
            break;
        case 270: {
            rotateNV21ToNv21(
                    (uint8_t *) src, (uint8_t *) dst, width, height, srcSize, libyuv::kRotate270);
        }
            break;
    }
    env->ReleaseByteArrayElements(nv21Src, src, 0);
    env->ReleaseByteArrayElements(nv21Output, dst, 0);

    return 0;
}

static jint
nativeRotateNv21ToI420(JNIEnv *env, jobject jclas, jbyteArray nv21Src, jbyteArray i420Output,
                       jint width, jint height,
                       jint rotation) {
    if (nv21Src == NULL || i420Output == NULL) {
        return INVALID_ARGS;
    }

    jsize srcSize = env->GetArrayLength(nv21Src);
    if (!CHECK_SIZE(width, height, PIXEL_NV21, srcSize)) {
        return DISMATCH_SIZE;
    }
    jsize dstSize = env->GetArrayLength(i420Output);
    if (!CHECK_SIZE(width, height, PIXEL_I420, dstSize)) {
        return DISMATCH_SIZE;
    }

    jbyte *src = env->GetByteArrayElements(nv21Src, NULL);
    jbyte *dst = env->GetByteArrayElements(i420Output, NULL);


    switch (rotation) {
        case 0: {
            rotateNV21ToI420(
                    (uint8_t *) src, (uint8_t *) dst, width, height, srcSize, libyuv::kRotate0);
        }
            break;
        case 90: {
            rotateNV21ToI420(
                    (uint8_t *) src, (uint8_t *) dst, width, height, srcSize, libyuv::kRotate90);
        }
            break;
        case 180: {
            rotateNV21ToI420(
                    (uint8_t *) src, (uint8_t *) dst, width, height, srcSize, libyuv::kRotate180);
        }
            break;
        case 270: {
            rotateNV21ToI420(
                    (uint8_t *) src, (uint8_t *) dst, width, height, srcSize, libyuv::kRotate270);
        }
            break;
    }
    env->ReleaseByteArrayElements(nv21Src, src, 0);
    env->ReleaseByteArrayElements(i420Output, dst, 0);

    return 0;
}


static jint nativeNv21ToRGB565(JNIEnv *env, jobject jclas,
                               jbyteArray src, jbyteArray output,
                               jint width, jint height) {
    LOGD("nv21ToRGB24");
    if (src == NULL || output == NULL) {
        return INVALID_ARGS;
    }

    jsize srcSize = env->GetArrayLength(src);
    if (!CHECK_SIZE(width, height, PIXEL_NV21, srcSize)) {
        return DISMATCH_SIZE;
    }

    uint8_t *i420 = NULL;
    i420 = (uint8_t *) malloc(width * height * PIXEL_I420);

    jbyte *src_nv21 = env->GetByteArrayElements(src, NULL);
    jbyte *output_rgb565 = env->GetByteArrayElements(output, NULL);

    nv21ToI420((uint8_t *) src_nv21, i420, width, height);
    //
    libyuv::I420ToRGB565(i420, width,
                       i420 + width * height, width >> 1,
                       i420 + width * height + (width >> 1) * (height >> 1), width >> 1,
                       (uint8_t *)output_rgb565, width * 2,
                       width, height);

    env->ReleaseByteArrayElements(src, src_nv21, 0);
    env->ReleaseByteArrayElements(output, output_rgb565, 0);
    free(i420);
    return 0;
}

static jint nativeNv21ToARGB8888(JNIEnv *env, jobject jclas,
                                 jbyteArray src, jbyteArray output,
                                 jint width, jint height) {

    if (src == NULL || output == NULL) {
        return INVALID_ARGS;
    }

    jsize srcSize = env->GetArrayLength(src);
    if (!CHECK_SIZE(width, height, PIXEL_NV21, srcSize)) {
        return DISMATCH_SIZE;
    }

    uint8_t *i420 = NULL;
    i420 = (uint8_t *) malloc(width * height * PIXEL_I420);

    jbyte *src_nv21 = env->GetByteArrayElements(src, NULL);
    jbyte *output_argb8888 = env->GetByteArrayElements(output, NULL);

    nv21ToI420((uint8_t *) src_nv21, i420, width, height);
    libyuv::I420ToABGR(i420, width,
                       i420 + width * height, width >> 1,
                       i420 + width * height + (width >> 1) * (height >> 1), width >> 1,
                       (uint8_t *)output_argb8888, width * 4,
                       width, height);

    env->ReleaseByteArrayElements(src, src_nv21, 0);
    env->ReleaseByteArrayElements(output, output_argb8888, 0);
    free(i420);
    return 0;
}

static JNINativeMethod methods[] = {
        {"rotateNV21ToNV21", "([B[BIII)I", (void *) nativeRotateNv21ToNv21},
        {"rotateNV21ToI420", "([B[BIII)I", (void *) nativeRotateNv21ToI420},
        {"nv21ToRGB565",     "([B[BII)I",  (void *) nativeNv21ToRGB565},
        {"nv21ToARGB8888",   "([B[BII)I",  (void *) nativeNv21ToARGB8888},
};

jint register_native_methods(JNIEnv *env, const char *class_name, JNINativeMethod *methods,
                             int num_methods) {
    int result = 0;
    jclass clazz = env->FindClass(class_name);

    // Find Class Success
    if (clazz) {
        result = env->RegisterNatives(clazz, methods, num_methods);
        if (result < 0) {
            LOGE("Register Natives Fail: %d", result);
        }
    } else {
        LOGE("Find Class Fail");
    }
    return result;
}

int register_jni(JNIEnv *env) {

    if (register_native_methods(env, CLASS_NAME, methods, ARRAY_LENGTH(methods)) < 0) {
        return -1;
    }
    return 0;
}
