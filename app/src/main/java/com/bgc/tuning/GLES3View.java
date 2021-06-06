package com.bgc.tuning;

import android.content.Context;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GLES3View extends GLSurfaceView {
    private static final String TAG = "GLES3JNI";
    private static final boolean DEBUG = true;

    public GLES3View(Context context) {
        super(context);
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        // supporting OpenGL ES 2.0 or later backwards-compatible versions.
        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(2);
        setRenderer(new Renderer(getContext()));
        //setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        private Context mContext;
        public Renderer(Context context) { mContext = context; }
        public void onDrawFrame(GL10 gl) {
            TuningLib.redraw();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            //TunaLib.resize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            TuningLib.init();
            //get default audio recording device ID
//            AudioManager audioManager = (AudioManager)
//                    mContext.getSystemService(Context.AUDIO_SERVICE);
//            AudioDeviceInfo deviceInfo =
//                    audioManager.getDevices(AudioManager.GET_DEVICES_INPUTS)[0];
            //TunaLib.setRecordingDevice(deviceInfo.getId());

        }
    }
}