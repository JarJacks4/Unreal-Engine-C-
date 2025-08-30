#pragma once

#include "CoreMinimal.h"

struct FEngineLoop;
extern "C" {
    void StartUnrealGame();
    void TickUnrealGame();
    void StopUnrealGame();
}
