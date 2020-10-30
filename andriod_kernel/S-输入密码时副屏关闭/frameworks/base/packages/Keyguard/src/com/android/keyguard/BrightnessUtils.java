package com.android.keyguard;
import android.content.SharedPreferences;
import android.util.Log;
import android.content.Context;
import android.os.AsyncTask;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;


/**
 * Created by Administrator on 2018/08/28
 */

public class BrightnessUtils {
    private static final String TAG = "yat";
    public static final String sys_path = "/sys/class/backlight/pwm3-backlight/brightness";//"/sys/class/backlight/backlight/brightness";
    public final static String BRIGHTNESSVALUES = "bright";

    public static void putValues(String key, String value ,Context context) {
        SharedPreferences.Editor sp = context.getSharedPreferences(BRIGHTNESSVALUES, context.MODE_PRIVATE).edit();
        sp.putString(key, value);
        sp.commit();
    }

    public static String getValues(String key, String defValue,Context context) {
        SharedPreferences sp = context.getSharedPreferences(BRIGHTNESSVALUES, context.MODE_PRIVATE);
        String value = sp.getString(key, defValue);
        return value;
    }



    public static String readSysFile(String sys_path) {

        if (sys_path == null || sys_path.length() == 0) return "";

        File file = new File(sys_path);
        if (!file.exists()) {
            Log.d(TAG, "file is not exits");
            return "";
        }
        String prop = "0";// 默认值
        if (file.isDirectory()) {
            Log.d(TAG, "The File is directory.");
        } else {

            BufferedReader reader = null;
            try {
                reader = new BufferedReader(new FileReader(sys_path));
                prop = reader.readLine();
                Log.d(TAG, "readFile: prop="+  prop);
            } catch (IOException e) {
                e.printStackTrace();
                Log.d(TAG, " ***ERROR*** Here is what I know: " + e.getMessage());
            } finally {
                if (reader != null) {
                    try {
                        reader.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        Log.d(TAG, "readFile cmd from" + sys_path + "data" + "  prop = " + prop);
        return prop;
    }


    public static void writeSysFile(String sys_path, String value) {

        if (sys_path == null || sys_path.length() == 0) return;

        File file = new File(sys_path);
        if (!file.exists()) {
            Log.d(TAG, "file is not exits");
            return;
        }

        if (file.isDirectory()) {
            Log.d(TAG, "The File is directory.");
        } else {
            BufferedWriter writer = null;
            try {

                writer = new BufferedWriter(new FileWriter(sys_path));
                writer.write(value);//写值操作
                writer.flush();
                writer.close();
                Log.d(TAG, "write value=" + value);
            } catch (IOException e) {
                e.printStackTrace();
                Log.d(TAG, "can't write the " + sys_path + e.toString());
            }
        }
    }
}