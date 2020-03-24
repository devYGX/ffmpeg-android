package org.libyuv.utils;

public class LibYuvUtils {
    static {
        System.loadLibrary("libyuv_utils");
    }

    /**
     * width: 640, height: 480
     * nv21ToI420 0, 1
     * nv21ToNv21 0, 1
     * nv21ToI420 90, 2
     * nv21ToNv21 90, 1
     * nv21ToI420 180, 1
     * nv21ToNv21 180, 1
     * nv21ToI420 270, 2
     * nv21ToNv21 270, 1
     *
     * @param src
     * @param output
     * @param width
     * @param height
     * @param rotation
     * @return
     */
    public native static int rotateNV21ToNV21(byte[] src, byte[] output, int width, int height,
                                              int rotation);

    /**
     * width: 640, height: 480
     * nv21ToI420 0, 1
     * nv21ToNv21 0, 1
     * nv21ToI420 90, 2
     * nv21ToNv21 90, 1
     * nv21ToI420 180, 1
     * nv21ToNv21 180, 1
     * nv21ToI420 270, 2
     * nv21ToNv21 270, 1
     *
     * @param src
     * @param output
     * @param width
     * @param height
     * @param rotation
     * @return
     */
    public native static int rotateNV21ToI420(byte[] src, byte[] output, int width, int height,
                                              int rotation);

    /**
     * NV21转成RGB565
     *
     * NV21 Buffer length: width * height * 3 / 2
     * RGB565 Buffer length: width * height * 2
     *
     * @param nv21 nv21 buffer
     * @param rgb565 rgb565 buffer
     * @param width 图片的高
     * @param height 图片的高
     * @return 转换的状态码: 0表示转换成功
     */
    public native static int nv21ToRGB565(byte[] nv21, byte[] rgb565, int width, int height);

    /**
     * NV21转成RGB565
     *
     * NV21 Buffer length: width * height * 3 / 2
     * ARGB8888 Buffer length: width * height * 4
     *
     * @param nv21 nv21 buffer
     * @param argb8888 argb8888 buffer
     * @param width 图片的高
     * @param height 图片的高
     * @return 转换的状态码: 0表示转换成功
     */
    public native static int nv21ToARGB8888(byte[] nv21, byte[] argb8888, int width, int height);

}
