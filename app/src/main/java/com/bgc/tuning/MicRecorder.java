package com.bgc.tuning;

// Based on code from https://stackoverflow.com/questions/4525206/android-audiorecord-class-process-live-mic-audio-quickly-set-up-callback-func

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

public class MicRecorder extends Thread {
    private boolean stopped    = false;
    @Override
    public void run() {
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
        AudioRecord recorder = null;
        final int BUFFER_SIZE = 8192;
        short[][]   buffers  = new short[256][BUFFER_SIZE];
//        short[] g_buffer = new short[4096];
        int         ix       = 0;
        try { // ... initialise
            int samplingRateHz = 44100;
            int N = AudioRecord.getMinBufferSize(samplingRateHz,AudioFormat.CHANNEL_IN_MONO,AudioFormat.ENCODING_PCM_16BIT);
            Log.i("audioin", "min samples="+N);
            recorder = new AudioRecord(MediaRecorder.AudioSource.CAMCORDER,
                    samplingRateHz,
                    AudioFormat.CHANNEL_IN_MONO,
                    AudioFormat.ENCODING_PCM_16BIT,
                    BUFFER_SIZE*2);
            recorder.setPositionNotificationPeriod(BUFFER_SIZE);
//            recorder.setRecordPositionUpdateListener(new AudioRecord.OnRecordPositionUpdateListener() {
//                @Override
//                public void onMarkerReached(AudioRecord audioRecord) {
//
//                }
//
//                @Override
//                public void onPeriodicNotification(AudioRecord audioRecord) {
//
//                }
//            });
            if (recorder.getState() == AudioRecord.STATE_UNINITIALIZED) {
                throw new Exception("recorder was not initialized");
            }
            recorder.startRecording();
            while(!stopped) {
                short[] buffer = buffers[ix++ % buffers.length];
                N = recorder.read(buffer,0,buffer.length);
                process(buffer);
            }
            recorder.stop();
            recorder.release();
        } catch(Throwable x) {
            Log.w("audioin","Error reading voice audio",x);
        } finally {
            close();
        }
    }

    private void process(short[] buffer) {
        TuningLib.updateFFT(buffer);
    }

    public void close() {
        stopped = true;
    }

}