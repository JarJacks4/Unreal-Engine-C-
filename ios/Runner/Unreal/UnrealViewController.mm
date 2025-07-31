#import "UnrealViewController.h"

// Required Unreal headers
#import "IOSAppDelegate.h"
#import "LaunchEngineLoop.h"
#import "IOS/IOSAppDelegate.h"
#import "SlateApplication.h"

@implementation UnrealViewController

- (instancetype)initWithLevel:(NSString *)levelName {
    self = [super init];
    if (self) {
        [self startUnrealWithLevel:levelName];
    }
    return self;
}

- (void)startUnrealWithLevel:(NSString *)levelName {
    static bool bEngineInitialized = false;

    if (!bEngineInitialized) {
        bEngineInitialized = true;

        // 1. Prepare arguments for UE (pass level name as command line arg)
        const TCHAR* CommandLine = FPlatformMisc::GetCommandLine();
        FString LaunchCmd = FString::Printf(TEXT("../../../YourUEProject/YourUEProject.uproject -game -level=%s"), *FString(levelName));
        FCommandLine::Set(*LaunchCmd);

        // 2. Launch UE Engine Loop
        static FEngineLoop GEngineLoop;
        GEngineLoop.PreInit(0, NULL);
        GEngineLoop.Init();

        // Optional: delay to ensure launch
        FPlatformProcess::Sleep(1.0f);
    }

    // 3. Attach the rendering view to this ViewController
    UIWindow* unrealWindow = [UIApplication sharedApplication].delegate.window;
    UIView* ueView = unrealWindow.rootViewController.view;

    [self.view addSubview:ueView];
    ueView.frame = self.view.bounds;
    ueView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
}

@end 