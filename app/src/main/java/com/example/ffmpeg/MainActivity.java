package com.example.ffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;

import cc.dewdrop.ffplayer.utils.FFUtils;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        Log.d(TAG, "onCreate:avCodecInfo "+FFUtils.avCodecInfo());
        Log.d(TAG, "onCreate:avFormatInfo "+FFUtils.avFormatInfo());
        Log.d(TAG, "onCreate:urlProtocolInfo "+FFUtils.urlProtocolInfo());
        Log.d(TAG, "onCreate:avFilterInfo "+FFUtils.avFilterInfo());

        //
        String code = code(new byte[]{0, 44, -48, 11});
        Log.d(TAG, "onCreate:code "+code);
    }

    private static String code(byte[] buf){
        String s = "";
        boolean canAbandon0 = true;
        int length = buf.length;
        for (int i = 0; i < length; i++) {
            if (buf[i] == 0 && canAbandon0) continue;
            canAbandon0 = false;
            String hex = Integer.toHexString(buf[i] & 0xFF);
            s += hex.length() < 2 ? "0" + hex : hex;
        }
        s = s.toUpperCase();
        return s;
    }
}
