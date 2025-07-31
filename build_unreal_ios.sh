#!/bin/bash
UE_PATH="/Users/rudravaishnav/Library/Application Support/Epic/UnrealEngine/5.6/Engine"
PROJECT_PATH="$(pwd)/Escape.uproject"
OUTPUT_DIR="$(pwd)/Unreal_iOS_Build"
"$UE_PATH/Build/BatchFiles/RunUAT.sh" BuildCookRun \
 -project="$PROJECT_PATH" \
 -noP4 -platform=IOS -clientconfig=Shipping -cook -allmaps -build \
 -stage -pak -archive -archivedirectory="$OUTPUT_DIR"
echo "Build complete. Output in $OUTPUT_DIR" 