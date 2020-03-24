package com.example.ffmpegsample;

import androidx.appcompat.app.AppCompatActivity;

import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import org.ffmpeg.utils.FFmpegUtils;
import org.ffmpeg.utils.StreamCallback;

import java.io.File;
import java.io.IOException;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {
    private static final String TAG = "MainActivity";
    private MediaPlayer player;
    private SurfaceHolder holder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
       //  FFmpegUtils.printFFmpegInfo();
        // FFmpegUtils.dumpMediaInfo(new File(Environment.getExternalStorageDirectory(), "test.mp4").getAbsolutePath());

        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(this);
    }

    boolean flag = false;

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        final Surface surface = holder.getSurface();
        Log.d(TAG, "surfaceCreated: ");
        this.holder = holder;
        play();
    }

    private void play(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                // FFmpegUtils.nativePlayVideo(new File(Environment.getExternalStorageDirectory(),"7.mp4").getAbsolutePath(),surface);
                //
                Log.d(TAG, "run: start ");
               //  FFmpegUtils.nativePlayStream("/sdcard/killer.mp4", holder.getSurface(), new StreamCallback() {
                FFmpegUtils.nativePlayStream("rtsp://admin:1234567890@10.0.0.39:554/mpeg4cif?username=admin&password=E10ADC3949BA59ABBE56E057F20F883E", holder.getSurface(), new StreamCallback() {
                    // FFmpegUtils.nativePlayStream("http://ivi.bupt.edu.cn/hls/cctv6hd.m3u8", surface, new StreamCallback() {
                    // FFmpegUtils.nativePlayStream("rtsp://192.168.2.156:8554/test2", surface, new StreamCallback() {
                    @Override
                    public void onStreamCallback(byte[] buffer, int width, int height) {
                        Log.d(TAG, "onStreamCallback: " + (buffer == null ? "null" : buffer.length) + ", " + width + ", " + height);

                    }
                });

                // FFmpegUtils.nativeSaveStreamMedia("rtmp://58.200.131.2:1935/livetv/hunantv",new File(Environment.getExternalStorageDirectory(),"baobei.flv").getAbsolutePath());

            }
        }).start();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void stopFFMPEG(View view) {
        FFmpegUtils.stopPlay();
    }

    public void startPlayStream(View view) {
        if(holder == null){
            return;
        }

        play();
    }
}
