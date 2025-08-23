#import "UnrealViewController.h"

// Unreal headers
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
    // Add a temporary loading screen
    self.view.backgroundColor = [UIColor blackColor];
    [self showLoadingScreen:levelName];

    static bool bEngineInitialized = false;
    if (!bEngineInitialized) {
        bEngineInitialized = true;

        // 1. Pass level as Unreal startup arg
        FString LaunchCmd = FString::Printf(
            TEXT("../../../YourUEProject/YourUEProject.uproject -game /Game/Levels/%s"),
            *FString(levelName)
        );
        FCommandLine::Set(*LaunchCmd);

        // 2. Start Unreal engine loop
        static FEngineLoop GEngineLoop;
        GEngineLoop.PreInit(0, NULL);
        GEngineLoop.Init();

        // Optional: small wait
        FPlatformProcess::Sleep(1.0f);
    }

    // 3. Attach Unreal rendering view
    UIWindow* unrealWindow = [UIApplication sharedApplication].delegate.window;
    UIView* ueView = unrealWindow.rootViewController.view;
    [self.view addSubview:ueView];
    ueView.frame = self.view.bounds;
    ueView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

    // 4. Bring loading screen UI back on top until ready
    [self.view bringSubviewToFront:self.loadingOverlay];
}

#pragma mark - Loading Screen

- (void)showLoadingScreen:(NSString *)levelName {
    // Keep a ref so we can remove it later
    self.loadingOverlay = [[UIView alloc] initWithFrame:self.view.bounds];
    self.loadingOverlay.backgroundColor = [UIColor colorWithWhite:0.05 alpha:1.0];
    [self.view addSubview:self.loadingOverlay];

    UILabel *titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 300, 50)];
    titleLabel.text = @"UNREAL ENGINE";
    titleLabel.textColor = [UIColor whiteColor];
    titleLabel.font = [UIFont boldSystemFontOfSize:24];
    titleLabel.textAlignment = NSTextAlignmentCenter;
    titleLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 100);
    [self.loadingOverlay addSubview:titleLabel];

    UILabel *levelLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 300, 30)];
    levelLabel.text = [NSString stringWithFormat:@"Loading: %@", levelName];
    levelLabel.textColor = [UIColor lightGrayColor];
    levelLabel.font = [UIFont systemFontOfSize:16];
    levelLabel.textAlignment = NSTextAlignmentCenter;
    levelLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 50);
    [self.loadingOverlay addSubview:levelLabel];

    // Progress bar
    UIView *progressBackground = [[UIView alloc] initWithFrame:CGRectMake(50, self.view.center.y + 50, 300, 4)];
    progressBackground.backgroundColor = [UIColor darkGrayColor];
    [self.loadingOverlay addSubview:progressBackground];

    UIView *progressBar = [[UIView alloc] initWithFrame:CGRectMake(50, self.view.center.y + 50, 0, 4)];
    progressBar.backgroundColor = [UIColor colorWithRed:0.2 green:0.6 blue:1.0 alpha:1.0];
    [self.loadingOverlay addSubview:progressBar];

    // Animate progress
    [UIView animateWithDuration:2.0 animations:^{
        progressBar.frame = CGRectMake(50, self.view.center.y + 50, 300, 4);
    } completion:^(BOOL finished) {
        [self.loadingOverlay removeFromSuperview]; // remove when done
    }];
}

#pragma mark - Close

- (void)closeGame {
    [self dismissViewControllerAnimated:YES completion:nil];
}

@end
