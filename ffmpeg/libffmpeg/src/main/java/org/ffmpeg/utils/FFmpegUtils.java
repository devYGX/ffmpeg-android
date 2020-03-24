package org.ffmpeg.utils;

import android.view.Surface;

public class FFmpegUtils {
    static {
        System.loadLibrary("ffmpeg_utils");
    }
    public native static void printFFmpegInfo();

    public native static int dumpMediaInfo(String file_path);

    /**
     * ffmpeg本地视频
     * @param file_path
     * @param surface
     */
    public native static void nativePlayVideo(String file_path, Surface surface);

    /**
     * ffmpeg播放rtsp, rtmp视频流
     * @param rtpUrl
     * @param surface
     * @param streamCallback
     */
    public native static void nativePlayStream(String rtpUrl, Surface surface, StreamCallback streamCallback);

    public native static void stopPlay();

    public native static void nativeSaveStreamMedia(String rtpUrl, String output_path);
}
