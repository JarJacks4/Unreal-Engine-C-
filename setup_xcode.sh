#!/bin/bash

echo "Setting up Xcode project for Unreal Engine integration..."

# Navigate to the iOS project directory
cd ios

# Open Xcode project
echo "Opening Xcode project..."
open Runner.xcworkspace

echo ""
echo "📋 Manual Xcode Configuration Required:"
echo ""
echo "1. In Xcode, go to Runner target → Build Settings"
echo "2. Search for 'Objective-C Bridging Header'"
echo "3. Set it to: Runner/Runner-Bridging-Header.h"
echo ""
echo "4. Search for 'Header Search Paths'"
echo "5. Add: \$(SRCROOT)/Runner/Unreal"
echo ""
echo "6. Go to Build Phases → Link Binary With Libraries"
echo "7. Add the Unreal Engine framework/library files"
echo ""
echo "8. In Build Settings, search for 'Other Linker Flags'"
echo "9. Add: -framework UnrealEngine"
echo ""
echo "🔧 Next Steps:"
echo "1. Build your Unreal project for iOS"
echo "2. Copy the built libraries to this project"
echo "3. Run: flutter run"
echo ""
echo "Press Enter when you've configured Xcode..."
read 