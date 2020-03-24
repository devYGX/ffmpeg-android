package org.ffmpeg.utils;

public interface StreamCallback {

    void onStreamCallback(byte[] buffer, int width, int height);
}
