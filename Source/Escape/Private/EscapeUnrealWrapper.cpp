#include "EscapeUnrealWrapper.h"
#include "LaunchEngineLoop.h"

static FEngineLoop GEngineLoopInstance;
static bool bEngineStarted = false;

extern "C" {
    void StartUnrealGame() {
        if (!bEngineStarted) {
            GEngineLoopInstance.PreInit(0, nullptr);
            GEngineLoopInstance.Init();
            bEngineStarted = true;
            UE_LOG(LogTemp, Display, TEXT("Unreal Engine started"));
        }
    }
    void TickUnrealGame() {
        if (bEngineStarted) {
            GEngineLoopInstance.Tick();
        }
    }
    void StopUnrealGame() {
        if (bEngineStarted) {
            GEngineLoopInstance.Exit();
            bEngineStarted = false;
            UE_LOG(LogTemp, Display, TEXT("Unreal Engine stopped"));
        }
    }
}
