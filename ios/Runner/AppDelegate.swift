import UIKit
import Flutter

@main
@objc class AppDelegate: FlutterAppDelegate {
    override func application(
        _ application: UIApplication,
        didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
    ) -> Bool {
        let controller : FlutterViewController = window?.rootViewController as! FlutterViewController
        let channel = FlutterMethodChannel(name: "unreal_bridge",
                                          binaryMessenger: controller.binaryMessenger)
        
        channel.setMethodCallHandler({
            (call: FlutterMethodCall, result: @escaping FlutterResult) -> Void in
            switch call.method {
            case "launchUnreal":
                self.launchUnreal(result: result)
            case "launchUnrealLevel":
                if let args = call.arguments as? [String: Any],
                   let levelName = args["levelName"] as? String {
                    self.launchUnrealLevel(levelName: levelName, result: result)
                } else {
                    result(FlutterError(code: "INVALID_ARGUMENTS", message: "levelName is required", details: nil))
                }
            case "launchEscapeInnerverse":
                self.launchEscapeInnerverse(result: result)
            case "isUnrealRunning":
                result(self.isUnrealRunning())
            case "stopUnreal":
                self.stopUnreal(result: result)
            default:
                result(FlutterMethodNotImplemented)
            }
        })
        
        return super.application(application, didFinishLaunchingWithOptions: launchOptions)
    }
    
    private func launchUnreal(result: @escaping FlutterResult) {
        DispatchQueue.main.async {
            // Launch the main Escape game level
            self.launchUnrealLevel(levelName: "MainMenu", result: result)
        }
    }
    
    private func launchEscapeInnerverse(result: @escaping FlutterResult) {
        DispatchQueue.main.async {
            // Launch the main Escape game level
            self.launchUnrealLevel(levelName: "MainMenu", result: result)
        }
    }
    
    private func launchUnrealLevel(levelName: String, result: @escaping FlutterResult) {
        DispatchQueue.main.async {
            // Create and present the Unreal Engine view controller
            let unrealVC = UnrealViewController(level: levelName)
            unrealVC.modalPresentationStyle = .fullScreen
            
            if let window = UIApplication.shared.keyWindow {
                window.rootViewController?.present(unrealVC, animated: true) {
                    result(true)
                }
            } else {
                result(FlutterError(code: "PRESENTATION_ERROR", message: "Could not present Unreal view", details: nil))
            }
        }
    }
    
    private func isUnrealRunning() -> Bool {
        // Check if Unreal Engine is currently running
        if let window = UIApplication.shared.keyWindow,
           let presentedVC = window.rootViewController?.presentedViewController {
            // Check if we have UnrealViewController presented
            return presentedVC is UnrealViewController
        }
        return false
    }
    
    private func stopUnreal(result: @escaping FlutterResult) {
        DispatchQueue.main.async {
            if let window = UIApplication.shared.keyWindow,
               let presentedVC = window.rootViewController?.presentedViewController,
               presentedVC is UnrealViewController {
                presentedVC.dismiss(animated: true) {
                    result(true)
                }
            } else {
                result(FlutterError(code: "NOT_RUNNING", message: "Unreal is not currently running", details: nil))
            }
        }
    }
}
