#import "EscapeUnrealWrapper.h"
#import "CoreMinimal.h"
#import "LaunchEngineLoop.h"
#import <EscapeUnrealFramework/EscapeUnrealFramework-Swift.h>
#import <UIKit/UIKit.h>

// Global pointer to Unreal view controller
static UIViewController* gUnrealViewController = nil;

// Unreal engine main loop pointer
static FEngineLoop* GEngineLoop = nullptr;

#ifdef __cplusplus
extern "C" {
#endif

// Start Unreal game
void StartUnrealGame(UIViewController* rootViewController)
{
    gUnrealViewController = rootViewController;

    // Call Swift framework main entry function
    [EscapeUnrealFrameworkMain()];

    // Initialize Unreal Engine loop
    GEngineLoop = new FEngineLoop();
    FString CommandLine = TEXT("-game -log");
    GEngineLoop->PreInit(*CommandLine);
    GEngineLoop->Init();
}

// Tick Unreal game per frame
void TickUnrealGame(float DeltaTime)
{
    if (GEngineLoop)
    {
        GEngineLoop->Tick();
    }
}

// Stop Unreal game
void StopUnrealGame()
{
    if (GEngineLoop)
    {
        GEngineLoop->Exit();
        delete GEngineLoop;
        GEngineLoop = nullptr;
    }

    if (gUnrealViewController)
    {
        [gUnrealViewController dismissViewControllerAnimated:YES completion:nil];
        gUnrealViewController = nil;
    }
}

#ifdef __cplusplus
}
#endif

