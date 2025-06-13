package com.epicgames.unreal;

import android.app.Activity;
import android.content.Intent;
import android.speech.RecognizerIntent;
import android.speech.SpeechRecognizer;
import android.speech.RecognitionListener;
import android.os.Bundle;
import java.util.ArrayList;

public class SpeechToTextHelper {
    static {
        try {
            System.loadLibrary("Escape"); // Replace with your actual .so name if different
        } catch (UnsatisfiedLinkError e) {
            android.util.Log.e("SpeechToTextHelper", "[DEBUG] Failed to load native library: " + e.getMessage());
        }
    }

    private static SpeechRecognizer speechRecognizer;
    private static Activity activity;

    public static void startSpeechToText() {
        activity = com.epicgames.unreal.GameActivity.Get();
        if (activity == null) return;

        // Request RECORD_AUDIO permission if needed (Android 6.0+)
        if (android.os.Build.VERSION.SDK_INT >= 23) {
            if (activity.checkSelfPermission(android.Manifest.permission.RECORD_AUDIO) != android.content.pm.PackageManager.PERMISSION_GRANTED) {
                activity.requestPermissions(new String[]{android.Manifest.permission.RECORD_AUDIO}, 1001);
                // You may want to handle the permission result in your activity for production use
                return;
            }
        }

        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {

                if (speechRecognizer == null) {
                    speechRecognizer = SpeechRecognizer.createSpeechRecognizer(activity);
                }
                Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
                intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
                intent.putExtra(RecognizerIntent.EXTRA_PROMPT, "Speak now...");
                speechRecognizer.setRecognitionListener(new RecognitionListener() {
                    @Override public void onReadyForSpeech(Bundle params) {}
                    @Override public void onBeginningOfSpeech() {}
                    @Override public void onRmsChanged(float rmsdB) {}
                    @Override public void onBufferReceived(byte[] buffer) {}
                    @Override public void onEndOfSpeech() {}
                    @Override public void onError(int error) {
                        nativeOnSpeechToTextResult("");
                    }
                    @Override public void onResults(Bundle results) {
                        ArrayList<String> matches = results.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION);
                        if (matches != null && matches.size() > 0) {
                            nativeOnSpeechToTextResult(matches.get(0));
                        } else {
                            nativeOnSpeechToTextResult("");
                        }
                    }
                    @Override public void onPartialResults(Bundle partialResults) {}
                    @Override public void onEvent(int eventType, Bundle params) {}
                });
                speechRecognizer.startListening(intent);
            }
        });
    }

    // Native method to call Unreal C++ side
    public static native void nativeOnSpeechToTextResult(String result);


}
