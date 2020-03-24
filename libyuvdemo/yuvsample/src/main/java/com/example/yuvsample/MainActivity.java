package com.example.yuvsample;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import org.libyuv.utils.LibYuvUtils;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private ImageView imageView;
    private ImageView imageView2;
    int width = 640;
    int height = 480;
    private final int REQUEST_CODE_PERMISSIONS = 10;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //权限申请使用
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            final String[] PERMISSIONS = new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE};
            PermissionsUtils.checkAndRequestMorePermissions(this, PERMISSIONS, REQUEST_CODE_PERMISSIONS,
                    new PermissionsUtils.PermissionRequestSuccessCallBack() {

                        @Override
                        public void onHasPermission() {

                            Log.d(TAG, "onHasPermission: ");
                        }
                    });
        }
        imageView = findViewById(R.id.imageView);
        imageView2 = findViewById(R.id.imageView2);

        InputStream inputStream = null;
        try {
            inputStream = getAssets().open("src/src.nv21");
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (inputStream != null) {
            byte[] nv21 = IOUtils.readAsBuffer(inputStream);

            testNv21TORGB565(nv21, width, height);
            testNv21TOARGB8888(nv21, width, height);

            int[] degrees = {0, 90, 180, 270};
            for (int degree : degrees) {
                byte[] output = new byte[width * height * 3 / 2];
                byte[] output_nv21 = new byte[width * height * 3 / 2];
                long start = System.currentTimeMillis();
                LibYuvUtils.rotateNV21ToI420(nv21, output, width, height, degree);
                Log.d(TAG, "onCreate: nv21ToI420 " + degree + ", " + (System.currentTimeMillis() - start));
                start = System.currentTimeMillis();
                LibYuvUtils.rotateNV21ToNV21(nv21, output_nv21, width, height, degree);
                Log.d(TAG, "onCreate: nv21ToNv21 " + degree + ", " + (System.currentTimeMillis() - start));
                FileUtils.write2File(getExternalFilesDir("I420"), degree + "_i420_rotate.i420", false, output);
                FileUtils.write2File(getExternalFilesDir("NV21"), degree + "_nv21_rotate.nv21", false, output_nv21);
            }
        }
    }

    private void testNv21TORGB565(byte[] nv21, int width, int height) {
        byte[] rgb565Buffer = new byte[height * width * 2];
        int i = LibYuvUtils.nv21ToRGB565(nv21, rgb565Buffer, width, height);
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
        bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(rgb565Buffer));
        imageView.setImageBitmap(bitmap);
    }

    private void testNv21TOARGB8888(byte[] nv21, int width, int height) {
        byte[] argb8888Buffer = new byte[height * width * 4];
        long start = System.currentTimeMillis();
        int i = LibYuvUtils.nv21ToARGB8888(nv21, argb8888Buffer, width, height);
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(argb8888Buffer));
        Log.d(TAG, "testNv21TOARGB8888: " + (System.currentTimeMillis() - start));
        // Bitmap bitmap = BitmapFactory.decodeByteArray(argb8888Buffer,0,argb8888Buffer.length);
        imageView2.setImageBitmap(bitmap);
    }

    private Bitmap nv21ToBitmap(byte[] yuv, int width, int height) {
        YuvImage yuvImage = new YuvImage(yuv, ImageFormat.NV21, width, height, null);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        yuvImage.compressToJpeg(new Rect(0, 0, width, height), 100, baos);
        byte[] byteArray = baos.toByteArray();
        Bitmap bitmap = BitmapFactory.decodeByteArray(byteArray, 0, byteArray.length);
        return bitmap;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
