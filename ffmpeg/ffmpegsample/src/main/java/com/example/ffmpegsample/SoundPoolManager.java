package com.example.ffmpegsample;

import android.content.Context;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.Log;
import android.util.SparseIntArray;

import androidx.annotation.RawRes;


/**
 * 音频池
 *
 * @author YGX
 */

public class SoundPoolManager {
    private static final String TAG = "SoundPoolManager";
    private final SoundPool soundPool;
    // private final SoundHandler soundHandler;
    private int streamType;
    private SparseIntArray mSoundIds = new SparseIntArray();
    private float mVolumeRatio;

    private SoundPoolManager() {

        streamType = AudioManager.STREAM_SYSTEM;
        soundPool = new SoundPool(5, streamType, 0);
        /*HandlerThread ht = new HandlerThread(TAG + "-Handler");
        ht.start();
        soundHandler = new SoundHandler(ht.getLooper());*/
    }

    private static SoundPoolManager sManager;

    public static SoundPoolManager getManager() {
        if (sManager == null) sManager = new SoundPoolManager();
        return sManager;
    }

/*
    static final int PLAY_RAW = 1;
    static final int ADJUST_VOLUME = 2;
    static final int PLAY_RAW_WITH_SPECIAL_VOLUME = 3;
    static final int RECOVER_VOLUME = 4;

    private class SoundHandler extends Handler {
        SoundHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case PLAY_RAW:
                    try {
                        List list = (List) msg.obj;
                        Context context = (Context) list.get(0);
                        int rawRes = (int) list.get(1);
                        playRawByHandler(context, rawRes);
                    } catch (Exception ignored) {
                    }
                    break;
                case ADJUST_VOLUME:
                    try {
                        List list = (List) msg.obj;
                        Context context = (Context) list.get(0);
                        float ratio = (float) list.get(1);
                        adjustVolumeByHandler(context, ratio);
                    } catch (Exception ignored) {
                    }
                    break;
                case PLAY_RAW_WITH_SPECIAL_VOLUME:
                    try {
                        List list = (List) msg.obj;
                        Context context = (Context) list.get(0);
                        int rawRes = (int) list.get(1);
                        float ratio = (float) list.get(2);
                        if (ratio < 0 || ratio > 1.0f) {
                            playRawByHandler(context, rawRes);
                        } else {
                            adjust2SpecialVolumeByHandler(context, ratio);
                            playRawByHandler(context, rawRes);
                        }
                    } catch (Exception ignored) {
                    }
                    break;
            }
        }
    }
*/

    private float getCurVolumeRatio(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        if (am != null) {
            int streamMaxVolume = am.getStreamMaxVolume(streamType);
            int streamVolume = am.getStreamVolume(streamType);
            return streamVolume * 1.0f / streamMaxVolume;
        } else {
            return -1;
        }
    }

    private void adjustVolumeByHandler(Context context, float ratio) {
        if (ratio > 1.0F || ratio < 0) {
            Log.d(TAG, "adjustVolume: illega arguement ratio: " + ratio);
            return;
        }
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        if (am != null) {
            int streamMaxVolume = am.getStreamMaxVolume(streamType);
            am.setStreamVolume(
                    streamType,
                    (int) (ratio * streamMaxVolume),
                    AudioManager.FLAG_PLAY_SOUND);//设置call的音量
            mVolumeRatio = ratio;
        }
    }

    private void adjust2SpecialVolumeByHandler(Context context, float ratio) {
        if (ratio > 1.0F || ratio < 0) {
            Log.d(TAG, "adjustVolume: illega arguement ratio: " + ratio);
            return;
        }
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        if (am != null) {
            int streamMaxVolume = am.getStreamMaxVolume(streamType);
            am.setStreamVolume(
                    streamType,
                    (int) (ratio * streamMaxVolume),
                    0);//设置call的音量
        }
    }

    private void playRawByHandler(Context context, final int rawRes) {
        int soundId = mSoundIds.get(rawRes);

        if (soundId == 0) {
            soundPool.load(context, rawRes, 1);
            soundPool.setOnLoadCompleteListener(new SoundPool.OnLoadCompleteListener() {
                @Override
                public void onLoadComplete(SoundPool soundPool, int sampleId, int status) {
                    mSoundIds.put(rawRes, sampleId);
                    soundPool.play(sampleId, 1, 1, 0, 0, 1);
                }
            });
        } else {
            soundPool.play(soundId, 1, 1, 0, 0, 1);
        }
    }

    public synchronized void loadRaw(Context context, @RawRes int rawRes) {
        if (mSoundIds.valueAt(rawRes) != -1) {
            int id = soundPool.load(context, rawRes, 1);
            if (id != 0) {
                mSoundIds.append(rawRes, id);
            } else {
                Log.d(TAG, "loadRaw: load raw fail ~ " + rawRes);
            }
        }
    }

    public synchronized void play(Context context, @RawRes int rawRes) {

        playRawByHandler(context, rawRes);
    }


    public synchronized void release() {
        int size = mSoundIds.size();
        for (int i = 0; i < size; i++) {
            int valueAt = mSoundIds.valueAt(i);
            soundPool.unload(valueAt);
        }
    }

    public synchronized void adjustVolume(Context context, float ratio) {
       /* Message message = Message.obtain(soundHandler);
        message.what = ADJUST_VOLUME;
        message.obj = Arrays.asList(context.getApplicationContext(), ratio);
        message.setTarget(soundHandler);
        message.sendToTarget();*/
        adjustVolumeByHandler(context, ratio);
    }
}
