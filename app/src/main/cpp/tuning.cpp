#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <EGL/egl.h>
#include <android/log.h>
//#include "gl3stub.h"
#include "nanovg.h"
#include "fftw3.h"
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"
#include <string>
#include <sstream>
#include "opensans_semibold_font.h"
#include "midi_notes.h"
// print out string with file, line and function
#define LOGI(x...) do { \
  char buf[512]; \
  sprintf(buf, x); \
  __android_log_print(ANDROID_LOG_INFO,"TAG", "%s | %s, %s:%i", buf, __PRETTY_FUNCTION__, \
  __FILE__, __LINE__); \
} while (0)
static EGLContext eglContext;
static NVGcontext* nvgContext;
static int BUFFER_LENGTH = 8192;
static fftw_complex *fft_output;
static fftw_plan my_plan;
static double* g_buffer;
static std::string g_ns;
[[maybe_unused]] static std::string g_fs;
static double g_maxFreqBin;
static double g_needle_pitch_offset = 0;
// draw tracker line with position relative to center
void drawTracker(NVGcontext* vg, float position=0)
{
    const float sx = 1, sy = 0.75;
    nvgSave(vg);
    nvgTranslate(vg, 0, 64);
    nvgScale(vg, sx, sy);
    const float vwh = 256, vhh = 256;
    //bottom triangle
    nvgBeginPath(vg);
    nvgMoveTo(vg, 301.758+(position*vwh), 429);
    nvgLineTo(vg, 220.664+(position*vwh), 429);
    nvgLineTo(vg, 261.211+(position*vwh), 360);
    nvgLineTo(vg, 301.758+(position*vwh), 429);
    nvgClosePath(vg);
    nvgFillColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgFill(vg);
    //line
    nvgBeginPath(vg);
    nvgMoveTo(vg, 261.211+(position*vwh), 139.81);
    nvgLineTo(vg, 261.211+(position*vwh), 359.142);
    nvgLineTo(vg, 261.211+(position*vwh), 139.81);
    nvgClosePath(vg);
//    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
//    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1);
    nvgStroke(vg);
    //top triangle
    nvgBeginPath(vg);
    nvgMoveTo(vg, 301.758+(position*vwh), 133.344);
    nvgLineTo(vg, 220.664+(position*vwh), 133.344);
    nvgLineTo(vg, 261.211+(position*vwh), 203.25);
    nvgLineTo(vg, 301.758+(position*vwh), 133.344);
    nvgClosePath(vg);
    nvgFillColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgFill(vg);
    nvgRestore(vg);
}

void drawNeedle(NVGcontext* context, double needle_offset=0)
{
    const float sx = 1, sy = 0.8;
    nvgSave(context);
    nvgScale(context, sx, sy);
    // TODO: rotate from a point at 0,5,1.0 on the needle
    const double angle = needle_offset * M_PI_2;
//    nvgTranslate(context, 258+cos(angle)*-136 - sin(angle * (-136)), 0);
    nvgSave(context);
    nvgTranslate(context,
                 258 + cos(angle) * (0 - 256) - sin(angle) * (0 - 321),
                 398  + sin(angle) * (0 - 256) + cos(angle) * (0 - 321));
    nvgRotate(context, angle);
    // needle
    nvgBeginPath(context);
    nvgMoveTo(context, 258.937, 155.259);
    nvgBezierTo(context, 258.937, 155.259, 268.119, 262.86199999999997, 268.119, 309.15599999999995);
    nvgBezierTo(context, 268.119, 315.02799999999996, 269.09700000000004, 321.46299999999997, 258.937, 321.46299999999997);
    nvgBezierTo(context, 248.77700000000002, 321.46299999999997, 249.74800000000002, 315.02799999999996, 249.75500000000002, 309.15599999999995);
    nvgBezierTo(context, 249.81, 259.694, 258.937, 155.259, 258.937, 155.259);
    nvgClosePath(context);
    nvgFillColor(context, nvgRGBA(216, 216, 216, 255));
//    nvgFillColor(context, nvgRGBA(216, 216, 0, 255));
    nvgFill(context);
    nvgRestore(context);
}


void drawTuner2(NVGcontext* context)
{
    const float sx = 1, sy = 0.55;
    const double LINE_WIDTH = 5;
    nvgSave(context);
    nvgTranslate(context, 0, 102);
    nvgScale(context, sx, sy);
    nvgBeginPath(context);
    nvgRect(context, 5.599, 3.847, 510.198, 513.592);
    nvgFillColor(context, nvgRGBA(0, 0, 0, 255));
    nvgFill(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 260.453, 106.973);
    nvgLineTo(context, 260.453, 136.367);
    nvgStrokeColor(context, nvgRGBA(0, 206, 209, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 318.622, 115.764);
    nvgLineTo(context, 309.875, 143.776);
    nvgStrokeColor(context, nvgRGBA(0, 139, 139, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 371.505, 142.152);
    nvgLineTo(context, 354.215, 165.896);
    nvgStrokeColor(context, nvgRGBA(34, 139, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 413.153, 183.789);
    nvgLineTo(context, 389.361, 201.101);
    nvgStrokeColor(context, nvgRGBA(34, 139, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 440.243, 235.947);
    nvgLineTo(context, 412.393, 245.345);
    nvgStrokeColor(context, nvgRGBA(34, 139, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 450.017, 294.1);
    nvgLineTo(context, 420.552, 294.1);
    nvgStrokeColor(context, nvgRGBA(34, 139, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 70.889, 294.1);
    nvgLineTo(context, 100.354, 294.1);
    nvgStrokeColor(context, nvgRGBA(178, 34, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 80.663, 235.946);
    nvgLineTo(context, 108.513, 245.344);
    nvgStrokeColor(context, nvgRGBA(178, 34, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 107.753, 183.788);
    nvgLineTo(context, 131.544, 201.101);
    nvgStrokeColor(context, nvgRGBA(178, 34, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 149.401, 142.151);
    nvgLineTo(context, 166.691, 165.895);
    nvgStrokeColor(context, nvgRGBA(178, 34, 34, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 202.284, 115.764);
    nvgLineTo(context, 211.031, 143.776);
    nvgStrokeColor(context, nvgRGBA(0, 139, 139, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 260.453, 106.973);
    nvgLineTo(context, 260.453, 136.367);
    nvgStrokeColor(context, nvgRGBA(0, 139, 139, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 368.718, 119.159);
    nvgBezierTo(context, 433.004, 157.965, 472.40700000000004, 228.317, 472.40700000000004, 304.289);
    nvgStrokeColor(context, nvgRGBA(124, 252, 0, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 141.663, 125.945);
    nvgBezierTo(context, 145.83700000000002, 123.07499999999999, 150.11100000000002, 120.35799999999999, 154.47500000000002, 117.797);
    nvgBezierTo(context, 220.05400000000003, 79.33, 300.851, 79.33, 366.43000000000006, 117.797);
    nvgBezierTo(context, 367.1960000000001, 118.246, 367.9580000000001, 118.7, 368.7180000000001, 119.15899999999999);
    nvgStrokeColor(context, nvgRGBA(0, 255, 255, 255));
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
    nvgBeginPath(context);
    nvgMoveTo(context, 48.498, 304.289);
    nvgBezierTo(context, 48.498, 232.817, 83.402, 166.00099999999998, 141.663, 125.945);
    nvgStrokeColor(context, nvgRGBA(255, 0, 0, 255));
    nvgLineCap(context, NVG_ROUND);
    nvgLineJoin(context, NVG_ROUND);
    nvgStrokeWidth(context, LINE_WIDTH);
    nvgStroke(context);
}

void drawWaterfallGraph(NVGcontext* vg)
{

}

// draw tuner bar
void drawTuner(NVGcontext* vg)
{
    const float sx = 1, sy = 0.75;
    nvgSave(vg);
    nvgTranslate(vg, 0, 64);
    nvgScale(vg, sx, sy);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 39.822, 293.505);
    nvgLineTo(vg, 485.348, 293.505);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgRect(vg, 71.347, 294.573, 379.471, 38.679);
    nvgFillColor(vg, nvgRGBA(109, 109, 109, 255));
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgRect(vg, 196.805, 294.573, 128.555, 38.679);
    nvgFillColor(vg, nvgRGBA(114, 255, 23, 255));
    nvgFill(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 261.211, 294.655);
    nvgLineTo(vg, 261.211, 333.10699999999997);
    nvgFillColor(vg, nvgRGBA(5, 9, 9, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(178, 223, 188, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 261.211, 256.202);
    nvgLineTo(vg, 261.211, 294.654);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 198.028, 256.202);
    nvgLineTo(vg, 198.028, 294.654);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 325.29, 256.202);
    nvgLineTo(vg, 325.29, 294.654);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 133.949, 256.202);
    nvgLineTo(vg, 133.949, 294.654);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 70.766, 256.202);
    nvgLineTo(vg, 70.766, 294.654);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 388.473, 256.202);
    nvgLineTo(vg, 388.473, 294.654);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgBeginPath(vg);
    nvgMoveTo(vg, 452.552, 256.202);
    nvgLineTo(vg, 452.552, 294.654);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgFill(vg);
    nvgStrokeColor(vg, nvgRGBA(160, 160, 160, 255));
    nvgStrokeWidth(vg, 1.3758);
    nvgStroke(vg);
    nvgRestore(vg);
}
// draw note letter above
void drawNoteLetter(NVGcontext* vg,
                    const char* text,
                    float x, float y,
                    float h = 60)
{
    const float sx = 1.0, sy = 1;
    nvgSave(vg);
    nvgScale(vg, sx, sy);
    nvgFontSize(vg, 128.f);
    nvgFontFace(vg, "sans");
    nvgFillColor(vg, nvgRGBA(160,160,160,255));
    nvgTextAlign(vg,NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
    nvgText(vg, x/sx,y/sy,text, NULL);
    nvgRestore(vg);
}
// use active gl context, create nvg context
bool initGraphics()
{
    eglContext = eglGetCurrentContext();
    nvgContext = nvgCreateGLES2(NVG_ANTIALIAS | NVG_DEBUG);
    nvgCreateFontMem(nvgContext, "sans", __OpenSans_SemiBold_ttf, 0, 0);
    return true;
}

bool initOther()
{
    fft_output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*BUFFER_LENGTH);
    g_buffer = new double[BUFFER_LENGTH];
    my_plan = fftw_plan_dft_r2c_1d(BUFFER_LENGTH, g_buffer, fft_output, FFTW_ESTIMATE);
    g_ns = "?";
    return true;
}

void hamming(double* in, int num_samples)
{
    // https://github.com/audacity/audacity/blob/c5ebc396eb06857b4509101fdd2b0620dc0658b3/src/FFT.cpp#L344
    const double multiplier = 2 * M_PI / num_samples;
    static const double coeff0 = 0.54, coeff1 = -0.46;
    for (int ii=0; ii < num_samples; ++ii) {
        in[ii] *= coeff0 + coeff1 * cos(ii * multiplier);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bgc_tuning_TuningLib_init(JNIEnv *env, jobject thiz) {
    LOGI("graphics");
    initGraphics();
    LOGI("other");
    initOther();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_bgc_tuning_TuningLib_redraw(JNIEnv *env, jobject thiz) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nvgBeginFrame(nvgContext, 512, 512, 1.0);
    drawTuner2(nvgContext);
    drawNeedle(nvgContext, g_needle_pitch_offset);
    //drawTracker(nvgContext, 0);
    drawNoteLetter(nvgContext, g_ns.c_str(), 126, 596);
    nvgEndFrame(nvgContext);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_bgc_tuning_TuningLib_cleanup(JNIEnv *env, jobject thiz) {
    if (eglGetCurrentContext() != eglContext)
        return;
    nvgDeleteGLES2(nvgContext);
    fftw_destroy_plan(my_plan);
    fftw_free(fft_output);
    delete g_buffer;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_bgc_tuning_TuningLib_updateFFT(JNIEnv *env, jclass clazz, jshortArray buffer) {
    jshort* data= (*env).GetShortArrayElements(buffer, 0);
    hamming(g_buffer, BUFFER_LENGTH);
    int i;
    // convert to double
    // get peak write out
    for (i=0; i < BUFFER_LENGTH; i++) {
        g_buffer[i] = (double)data[i]/std::numeric_limits<short>::max();
    }
    fftw_execute(my_plan);
    double bg = 0;
    double freqBinSize = (double)44100 / BUFFER_LENGTH;
    double b = 0;
    g_maxFreqBin = 0;
    for (i=0; i<BUFFER_LENGTH;i++) {
        double freq = sqrt(pow(fft_output[i][0], 2) + pow(fft_output[i][1],
                                                          2));
        if (freq > bg) {
            bg = freq;
            g_maxFreqBin = b;
        }
        b += freqBinSize;
    }
    std::ostringstream strs;
    const int MAX_FREQ_VARIANCE = freqBinSize;
    bool foundPitch = false;
    for (i=0; i < sizeof(MIDI_NOTES) / sizeof(MidiNote); i++) {
        if (g_maxFreqBin > MIDI_NOTES[i].freq - MAX_FREQ_VARIANCE && g_maxFreqBin < MIDI_NOTES[i].freq +
                                                                                    MAX_FREQ_VARIANCE) {
            foundPitch = true;
            strs << MIDI_NOTES[i].note;
            g_needle_pitch_offset = (g_maxFreqBin - g_maxFreqBin - MIDI_NOTES[i].freq) / (MAX_FREQ_VARIANCE);
            __android_log_print(ANDROID_LOG_INFO, "TAG", "g_needle_pitch_offset=%f", g_needle_pitch_offset);
            break;
        }
    }
    // if a pitch wasn't found then just display dashes
    if (!foundPitch) {
        strs << "--";
    }
    //strs << '\n' << mfb << " Hz";
    g_ns = strs.str();
    strs.clear();
//    __android_log_print(ANDROID_LOG_INFO, "TAG", "%s", g_ns.c_str());
//    drawNoteLetter(nvgContext, g_ns.c_str(), 0, 0);
    __android_log_print(ANDROID_LOG_INFO, "TAG", "mfb=%f", g_maxFreqBin);
//    LOGI("mfb=", mfb);
}