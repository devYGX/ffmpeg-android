package com.example.yuvsample;

import android.content.Context;
import android.graphics.Bitmap;
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * 操作文件的
 *
 * @author YGX
 */

public class FileUtils {
    private static final String TAG = "FileUtils";

    @NonNull
    public static final void copyFileFromAssetsToOthers(@NonNull final Context context, @NonNull final String fileName, @NonNull final String targetPath) {
        InputStream in = null;
        FileOutputStream out = null;
        try {
            in = context.getAssets().open(fileName);
            out = new FileOutputStream(targetPath);
            byte[] buff = new byte[1024];
            int read = 0;
            while ((read = in.read(buff)) > 0) {
                out.write(buff, 0, read);
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
                if (out != null) {
                    out.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    public static void zipFiles(File zipFile, File... srcs) throws IOException {
        ZipOutputStream zos = null;
        try {
            if (!zipFile.exists()) {
                zipFile.createNewFile();
            }
            zos = new ZipOutputStream(new FileOutputStream(zipFile));
            for (File f : srcs) {
                zipFile(f, zos, "");
            }
        } catch (IOException e) {
            throw e;
        } finally {
            if (zos != null) {
                try {
                    zos.close();
                } catch (IOException e) {
                }
            }
        }
    }

    private static void zipFile(File src, ZipOutputStream zos, String rootpath)
            throws IOException {
        rootpath = rootpath + (rootpath.trim().length() == 0 ? "" : File.separator)
                + src.getName();
        if (src.isDirectory()) {
            File[] fileList = src.listFiles();
            for (File file : fileList) {
                zipFile(file, zos, rootpath);
            }
        } else {
            byte buffer[] = new byte[1024];
            BufferedInputStream in = new BufferedInputStream(new FileInputStream(src), 1024);
            zos.putNextEntry(new ZipEntry(rootpath));
            int len = -1;
            while ((len = in.read(buffer)) != -1) {
                zos.write(buffer, 0, len);
            }
            in.close();
            zos.flush();
            zos.closeEntry();
        }
    }

    public static void delete(File file) {
        if (file == null) return;
        if (!file.exists()) return;
        if (file.isFile()) {
            file.delete();
        } else {
            File[] files = file.listFiles();
            if (files != null && files.length != 0) {
                for (File f : files) {
                    delete(f);
                }
            }
            file.delete();
        }
    }

    public static String loadAsString(File file) {
        if (file == null || !file.exists() || file.isDirectory()) return null;
        InputStream is = null;
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
            is = new FileInputStream(file);
            int len = -1;
            byte[] buf = new byte[64];
            while ((len = is.read(buf)) != -1) {
                baos.write(buf, 0, len);
            }
            return baos.toString();
        } catch (Exception ignored) {

        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                }
            }
        }
        return null;
    }

    public static byte[] loadAsBuffer(File file) {
        if (file == null || !file.exists() || file.isDirectory()) return null;
        InputStream is = null;
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
            is = new FileInputStream(file);
            int len = -1;
            byte[] buf = new byte[64];
            while ((len = is.read(buf)) != -1) {
                baos.write(buf, 0, len);
            }
            return baos.toByteArray();
        } catch (Exception ignored) {

        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                }
            }
        }
        return null;
    }

    public static String getFileMD5(File file) throws Exception {
        if (!file.isFile()) {
            return null;
        }
        MessageDigest digest = null;
        FileInputStream in = null;
        byte[] buffer = new byte[1024];
        int len;
        try {
            digest = MessageDigest.getInstance("MD5");
            in = new FileInputStream(file);
            while ((len = in.read(buffer, 0, 1024)) != -1) {
                digest.update(buffer, 0, len);
            }
            in.close();
        } catch (Exception e) {
            throw e;
        }
        byte[] digestBuffer = digest.digest();
        StringBuilder stringBuilder = new StringBuilder();
        for (int i = 0; i < digestBuffer.length; i++) {
            int v = digestBuffer[i] & 0xFF;
            String hv = Integer.toHexString(v);
            if (hv.length() < 2) {
                stringBuilder.append(0);
            }
            stringBuilder.append(hv);
        }
        return stringBuilder.toString();
    }

    public static void write2File(File file, String s) {
        try (OutputStream os = new FileOutputStream(file)) {
            os.write(s.getBytes());
            os.flush();
        } catch (Exception e) {

        }
    }

    public static void write2File(File targetDir, String name, boolean append, byte[] buf) {

        if (targetDir == null || TextUtils.isEmpty(name) || buf == null) {
            Log.d(TAG, "write2File, someone args is null");
            return;
        }

        if (!targetDir.exists()) {
            targetDir.mkdir();
        }
        try (OutputStream os = new FileOutputStream(new File(targetDir, name), append)) {
            os.write(buf);
            os.flush();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void write2File(File targetDir, String name, boolean append, String... str) {

        if (targetDir == null || TextUtils.isEmpty(name) || str == null) {
            Log.d(TAG, "write2File, someone args is null");
            return;
        }

        if (!targetDir.exists()) {
            targetDir.mkdir();
        }
        try (OutputStream os = new FileOutputStream(new File(targetDir, name), append)) {
            for (String s : str) {
                os.write(s.getBytes());
                os.flush();
            }
        } catch (Exception e) {

        }
    }

    public static void cap(File src, File targetDir, String name) {
        if (src == null || targetDir == null || TextUtils.isEmpty(name)) {
            return;
        }

        if (!targetDir.exists()) {
            targetDir.mkdir();
        }

        try (InputStream is = new FileInputStream(src);
             OutputStream os = new FileOutputStream(new File(targetDir, name))) {
            byte[] buf = new byte[1024];
            int len = -1;

            while ((len = is.read(buf)) != -1) {
                os.write(buf, 0, len);
                os.flush();
            }
            FileUtils.delete(src);
        } catch (Exception ignored) {
        }
    }


    public static void copy(File src, File targetDir, String name) {
        if (src == null || targetDir == null || TextUtils.isEmpty(name)) {
            return;
        }

        if (!targetDir.exists()) {
            targetDir.mkdir();
        }

        try (InputStream is = new FileInputStream(src);
             OutputStream os = new FileOutputStream(new File(targetDir, name))) {
            byte[] buf = new byte[1024];
            int len = -1;

            while ((len = is.read(buf)) != -1) {
                os.write(buf, 0, len);
                os.flush();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void savePic(File file, Bitmap bmp) throws IOException {
        OutputStream os = null;
        try {

            os = new FileOutputStream(file, false);
            bmp.compress(Bitmap.CompressFormat.JPEG, 100, os);
            os.flush();
        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        } finally {
            if (os != null) {
                try {
                    os.close();
                } catch (IOException ignored) {
                }
            }
        }
    }
}
