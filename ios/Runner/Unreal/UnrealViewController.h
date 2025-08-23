#import <UIKit/UIKit.h>

@interface UnrealViewController : UIViewController

// Callback for when Unreal finishes or user exits Animus
@property (nonatomic, copy) void (^onExit)(void);

@end
 
