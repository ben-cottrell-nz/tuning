package com.bgc.tuning;

public class TuningLib {
    static {
        System.loadLibrary("tuning");
    }
    //public native String stringFromJNI();
    public static native void init();
    public static native void redraw();
    public static native void cleanup();
    public static native void updateFFT(short[] buffer);
}
