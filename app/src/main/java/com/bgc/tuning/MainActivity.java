package com.bgc.tuning;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;

public class MainActivity extends AppCompatActivity {
    GLES3View mView;
    MicRecorder mRecorder;
    private static final int REQUEST_RECORD_AUDIO_PERMISSION = 200;
    private boolean permissionToRecordAccepted = false;
    private final String [] permissions = {Manifest.permission.RECORD_AUDIO};

    void restartAudioEngine()
    {
        if (permissionToRecordAccepted) {
            mRecorder.close();
            TuningLib.cleanup();
            mRecorder = new MicRecorder();
            mRecorder.start();
        }
    }
    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode){
            case REQUEST_RECORD_AUDIO_PERMISSION:
                permissionToRecordAccepted  = grantResults[0] == PackageManager.PERMISSION_GRANTED;
                mRecorder = new MicRecorder();
                mRecorder.start();
                break;
        }
        if (!permissionToRecordAccepted ) finish();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mView = new GLES3View(getApplicationContext());
        //GLSurfaceView surfaceView = findViewById(R.id.GLES3View);
        setContentView(R.layout.activity_main);
        FrameLayout frameLayout = findViewById(R.id.frameLayout);
        frameLayout.addView(mView);
        Button buttonRestartAudioEngine = findViewById(R.id.buttonRestartAudioEngine);
        buttonRestartAudioEngine.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                restartAudioEngine();
            }
        });
        // get permission
        ActivityCompat.requestPermissions(this, permissions, REQUEST_RECORD_AUDIO_PERMISSION);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mRecorder.close();
        TuningLib.cleanup();
    }
}