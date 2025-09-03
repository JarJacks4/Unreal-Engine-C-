import Flutter
import UIKit

public class UnrealPlugin: NSObject, FlutterPlugin {
    public static func register(with registrar: FlutterPluginRegistrar) {
        let channel = FlutterMethodChannel(name: "unreal_bridge", binaryMessenger: registrar.messenger())
        let instance = UnrealPlugin()
        registrar.addMethodCallDelegate(instance, channel: channel)
    }
    
    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
        case "launchUnrealLevel":
            if let args = call.arguments as? [String: Any],
               let levelName = args["levelName"] as? String {
                launchUnrealLevel(levelName: levelName, result: result)
            } else {
                result(FlutterError(code: "INVALID_ARGUMENTS", message: "levelName is required", details: nil))
            }
            
        case "isUnrealRunning":
            result(true) // Placeholder - implement actual check
            
        case "stopUnreal":
            stopUnreal(result: result)
            
        default:
            result(FlutterMethodNotImplemented)
        }
    }
    
    private func launchUnrealLevel(levelName: String, result: @escaping FlutterResult) {
        DispatchQueue.main.async {
            let unrealVC = UnrealViewController(levelName: levelName)
            if let windowScene = UIApplication.shared.connectedScenes.first as? UIWindowScene,
               let window = windowScene.windows.first {
                window.rootViewController = unrealVC
                window.makeKeyAndVisible()
                result(true)
            } else {
                result(FlutterError(code: "NO_WINDOW", message: "Could not find window", details: nil))
            }
        }
    }
    
    private func stopUnreal(result: @escaping FlutterResult) {
        // Implement stop logic
        result(true)
    }
} 