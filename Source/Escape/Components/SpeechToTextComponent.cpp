#include "SpeechToTextComponent.h"
#include "Misc/MessageDialog.h"
#include "Engine.h"
#include "Kismet/GameplayStatics.h"


#include "UObject/UObjectIterator.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include <jni.h>
#include <android/log.h>
#include "Async/AsyncWork.h"
#endif

// Forward declaration for iOS
#if PLATFORM_IOS
#include "IOS/IOSAppDelegate.h"
#include "Async/AsyncWork.h"
extern "C" void StartIOSSpeechRecognition();
extern "C" void StopIOSSpeechRecognition();
#endif

void USpeechToTextComponent::BeginPlay(){
    Super::BeginPlay();
    AudioTextHolder = GetWorld()->SpawnActor<AChatAudioTextHolder>();

}
void USpeechToTextComponent::StartSpeechToText()
{

#if PLATFORM_ANDROID
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (!Env)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[DEBUG] Failed to get Java environment (ToSpeechToText)"));
        return;
    }
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[DEBUG] Java environment obtained (ToSpeechToText)"));
    jclass Class = FAndroidApplication::FindJavaClass("com/epicgames/unreal/SpeechToTextHelper");
    if (!Class)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[DEBUG] Failed to get Java class (ToSpeechToText)"));
        return;
    }
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[DEBUG] Java class obtained (ToSpeechToText)"));
    jmethodID Method = Env->GetStaticMethodID(Class, "startSpeechToText", "()V");
    if (Method)
    {
        Env->CallStaticVoidMethod(Class, Method);
    }
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[DEBUG] Java Method obtained (ToSpeechToText)"));
    
    Env->DeleteLocalRef(Class);
#endif

#if PLATFORM_IOS
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[DEBUG] iOS Speech Recognition Started"));
    StartIOSSpeechRecognition();
#endif
}

void USpeechToTextComponent::OnSpeechToTextResult(FString Result)
{
    AChatAudioTextHolder* DeepBreathingComponent =  nullptr;
    for (TObjectIterator<AChatAudioTextHolder> It; It; ++It) {
        DeepBreathingComponent = *It;
    }
    DeepBreathingComponent->IWSetText(Result);

}

#if PLATFORM_ANDROID
// JNI callback from Java
extern "C" JNIEXPORT void JNICALL
Java_com_epicgames_unreal_SpeechToTextHelper_nativeOnSpeechToTextResult(JNIEnv* jenv, jclass clazz, jstring result)
{
    __android_log_print(ANDROID_LOG_DEBUG, "UE_SpeechToText", "[DEBUG] JNI nativeOnSpeechToTextResult called");
    FString SpeechResult;
    if (result != nullptr) {
        const char* UTFString = jenv->GetStringUTFChars(result, 0);
        SpeechResult = UTF8_TO_TCHAR(UTFString);
        jenv->ReleaseStringUTFChars(result, UTFString);
    } else {
        SpeechResult = TEXT("");
    }

    // Dispatch to game thread to safely update UI
    AsyncTask(ENamedThreads::GameThread, [SpeechResult]()
    {
        USpeechToTextComponent::OnSpeechToTextResult(SpeechResult);
    });
}
#endif

#if PLATFORM_IOS
// iOS callback function - called from Objective-C++
extern "C" void OnIOSSpeechResult(const char* result)
{
    FString SpeechResult = UTF8_TO_TCHAR(result ? result : "");
    
    // Dispatch to game thread to safely update UI
    AsyncTask(ENamedThreads::GameThread, [SpeechResult]()
    {
        USpeechToTextComponent::OnSpeechToTextResult(SpeechResult);
    });
}
#endif

void USpeechToTextComponent::StopSpeechToText()
{
#if PLATFORM_ANDROID
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[DEBUG] Android Speech Recognition Stopped"));
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
    if (Env)
    {
        jclass Class = FAndroidApplication::FindJavaClass("com/epicgames/unreal/SpeechToTextHelper");
        if (Class)
        {
            jmethodID Method = Env->GetStaticMethodID(Class, "stopSpeechToText", "()V");
            if (Method)
            {
                Env->CallStaticVoidMethod(Class, Method);
            }
            Env->DeleteLocalRef(Class);
        }
    }
#endif

#if PLATFORM_IOS
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[DEBUG] iOS Speech Recognition Stopped"));
    StopIOSSpeechRecognition();
#endif
}
