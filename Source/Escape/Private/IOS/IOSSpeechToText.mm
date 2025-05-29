#if PLATFORM_IOS

// Ensure ARC is enabled for this file
#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag.
#endif

#import <Foundation/Foundation.h>
#import <Speech/Speech.h>
#import <AVFoundation/AVFoundation.h>

// Forward declaration of the C++ callback function
extern "C" void OnIOSSpeechResult(const char* result);

@interface IOSSpeechRecognizer : NSObject <SFSpeechRecognizerDelegate>
@property (nonatomic, strong) SFSpeechRecognizer *speechRecognizer;
@property (nonatomic, strong) SFSpeechAudioBufferRecognitionRequest *recognitionRequest;
@property (nonatomic, strong) SFSpeechRecognitionTask *recognitionTask;
@property (nonatomic, strong) AVAudioEngine *audioEngine;

- (void)startRecording;
- (void)stopRecording;
@end

@implementation IOSSpeechRecognizer

- (instancetype)init {
    self = [super init];
    if (self) {
        self.speechRecognizer = [[SFSpeechRecognizer alloc] initWithLocale:[NSLocale localeWithLocaleIdentifier:@"en-US"]];
        self.speechRecognizer.delegate = self;
        self.audioEngine = [[AVAudioEngine alloc] init];
    }
    return self;
}

- (void)startRecording {
    // Check authorization
    [SFSpeechRecognizer requestAuthorization:^(SFSpeechRecognizerAuthorizationStatus authStatus) {
        dispatch_async(dispatch_get_main_queue(), ^{
            switch (authStatus) {
                case SFSpeechRecognizerAuthorizationStatusAuthorized:
                    [self beginRecognition];
                    break;
                case SFSpeechRecognizerAuthorizationStatusDenied:
                case SFSpeechRecognizerAuthorizationStatusRestricted:
                case SFSpeechRecognizerAuthorizationStatusNotDetermined:
                    OnIOSSpeechResult("Speech recognition not authorized");
                    break;
            }
        });
    }];
}

- (void)beginRecognition {
    if (self.recognitionTask) {
        [self.recognitionTask cancel];
        self.recognitionTask = nil;
    }
    
    NSError *error;
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    [audioSession setCategory:AVAudioSessionCategoryRecord error:&error];
    [audioSession setMode:AVAudioSessionModeMeasurement error:&error];
    [audioSession setActive:YES withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation error:&error];
    
    if (error) {
        OnIOSSpeechResult("Audio session setup failed");
        return;
    }
    
    self.recognitionRequest = [[SFSpeechAudioBufferRecognitionRequest alloc] init];
    
    AVAudioInputNode *inputNode = self.audioEngine.inputNode;
    if (!inputNode) {
        OnIOSSpeechResult("Audio engine has no input node");
        return;
    }
    
    if (!self.recognitionRequest) {
        OnIOSSpeechResult("Unable to create recognition request");
        return;
    }
    
    self.recognitionRequest.shouldReportPartialResults = YES;
    
    self.recognitionTask = [self.speechRecognizer recognitionTaskWithRequest:self.recognitionRequest
                                                                resultHandler:^(SFSpeechRecognitionResult * _Nullable result, NSError * _Nullable error) {
        BOOL isFinal = NO;
        
        if (result) {
            NSString *transcription = result.bestTranscription.formattedString;
            OnIOSSpeechResult([transcription UTF8String]);
            isFinal = result.isFinal;
        }
        
        if (error || isFinal) {
            [self.audioEngine stop];
            [inputNode removeTapOnBus:0];
            
            self.recognitionRequest = nil;
            self.recognitionTask = nil;
        }
    }];
    
    AVAudioFormat *recordingFormat = [inputNode outputFormatForBus:0];
    [inputNode installTapOnBus:0 bufferSize:1024 format:recordingFormat block:^(AVAudioPCMBuffer * _Nonnull buffer, AVAudioTime * _Nonnull when) {
        [self.recognitionRequest appendAudioPCMBuffer:buffer];
    }];
    
    [self.audioEngine prepare];
    [self.audioEngine startAndReturnError:&error];
    
    if (error) {
        OnIOSSpeechResult("Audio engine failed to start");
    }
}

- (void)stopRecording {
    [self.audioEngine stop];
    [self.recognitionRequest endAudio];
}

@end

static IOSSpeechRecognizer *globalSpeechRecognizer = nil;

extern "C" void StartIOSSpeechRecognition() {
    if (!globalSpeechRecognizer) {
        globalSpeechRecognizer = [[IOSSpeechRecognizer alloc] init];
    }
    [globalSpeechRecognizer startRecording];
}

extern "C" void StopIOSSpeechRecognition() {
    if (globalSpeechRecognizer) {
        [globalSpeechRecognizer stopRecording];
    }
}

#endif // PLATFORM_IOS
