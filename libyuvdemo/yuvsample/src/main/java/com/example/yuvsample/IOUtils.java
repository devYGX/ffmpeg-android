package com.example.yuvsample;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class IOUtils {

    public static byte[] readAsBuffer(InputStream is) {
        if (is == null) {
            return null;
        }
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        byte[] buf = new byte[64];
        int len = -1;

        try {
            while ((len = is.read(buf)) != -1) {
                baos.write(buf, 0, len);
            }
            return baos.toByteArray();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                is.close();
            } catch (IOException ignored) {
            }
        }
        return null;
    }

    public static void write2File(byte[] buffer, File f){
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(f);
            fos.write(buffer);
            fos.close();
        }catch (Exception e){}
        finally {
            if(fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
