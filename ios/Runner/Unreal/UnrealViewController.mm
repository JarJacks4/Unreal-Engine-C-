#import "UnrealViewController.h"
#import <UIKit/UIKit.h>

@implementation UnrealViewController

- (instancetype)initWithLevel:(NSString *)levelName {
    self = [super init];
    if (self) {
        [self startUnrealWithLevel:levelName];
    }
    return self;
}

- (void)startUnrealWithLevel:(NSString *)levelName {
    // Create a realistic Unreal Engine game interface
    self.view.backgroundColor = [UIColor blackColor];
    
    // Add a loading screen effect
    UIView *loadingView = [[UIView alloc] initWithFrame:self.view.bounds];
    loadingView.backgroundColor = [UIColor colorWithRed:0.1 green:0.1 blue:0.1 alpha:1.0];
    [self.view addSubview:loadingView];
    
    // Unreal Engine logo/loading text
    UILabel *titleLabel = [[UILabel alloc] init];
    titleLabel.text = @"UNREAL ENGINE";
    titleLabel.textColor = [UIColor whiteColor];
    titleLabel.font = [UIFont boldSystemFontOfSize:24];
    titleLabel.textAlignment = NSTextAlignmentCenter;
    titleLabel.frame = CGRectMake(0, 0, 300, 50);
    titleLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 100);
    [loadingView addSubview:titleLabel];
    
    // Level name
    UILabel *levelLabel = [[UILabel alloc] init];
    levelLabel.text = [NSString stringWithFormat:@"Loading: %@", levelName];
    levelLabel.textColor = [UIColor lightGrayColor];
    levelLabel.font = [UIFont systemFontOfSize:16];
    levelLabel.textAlignment = NSTextAlignmentCenter;
    levelLabel.frame = CGRectMake(0, 0, 300, 30);
    levelLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 50);
    [loadingView addSubview:levelLabel];
    
    // Progress bar
    UIView *progressBar = [[UIView alloc] initWithFrame:CGRectMake(50, 0, 0, 4)];
    progressBar.backgroundColor = [UIColor colorWithRed:0.2 green:0.6 blue:1.0 alpha:1.0];
    progressBar.center = CGPointMake(self.view.center.x, self.view.center.y + 50);
    [loadingView addSubview:progressBar];
    
    // Progress bar background
    UIView *progressBackground = [[UIView alloc] initWithFrame:CGRectMake(50, 0, 300, 4)];
    progressBackground.backgroundColor = [UIColor colorWithRed:0.3 green:0.3 blue:0.3 alpha:1.0];
    progressBackground.center = CGPointMake(self.view.center.x, self.view.center.y + 50);
    [loadingView addSubview:progressBackground];
    [loadingView sendSubviewToBack:progressBackground];
    
    // Animate the progress bar
    [UIView animateWithDuration:2.0 delay:0.5 options:UIViewAnimationOptionCurveEaseInOut animations:^{
        progressBar.frame = CGRectMake(50, 0, 300, 4);
    } completion:^(BOOL finished) {
        // Simulate game loading completion
        [self showGameInterface:levelName];
    }];
    
    NSLog(@"UnrealViewController: Started with level %@", levelName);
}

- (void)showGameInterface:(NSString *)levelName {
    // Remove loading view
    for (UIView *subview in self.view.subviews) {
        [subview removeFromSuperview];
    }
    
    // Create game interface
    self.view.backgroundColor = [UIColor colorWithRed:0.05 green:0.05 blue:0.1 alpha:1.0];
    
    // Game title
    UILabel *gameTitle = [[UILabel alloc] init];
    gameTitle.text = @"ESCAPE INNERVERSE";
    gameTitle.textColor = [UIColor whiteColor];
    gameTitle.font = [UIFont boldSystemFontOfSize:28];
    gameTitle.textAlignment = NSTextAlignmentCenter;
    gameTitle.frame = CGRectMake(0, 0, 350, 40);
    gameTitle.center = CGPointMake(self.view.center.x, 100);
    [self.view addSubview:gameTitle];
    
    // Level info
    UILabel *levelInfo = [[UILabel alloc] init];
    levelInfo.text = [NSString stringWithFormat:@"Level: %@", levelName];
    levelInfo.textColor = [UIColor lightGrayColor];
    levelInfo.font = [UIFont systemFontOfSize:18];
    levelInfo.textAlignment = NSTextAlignmentCenter;
    levelInfo.frame = CGRectMake(0, 0, 300, 30);
    levelInfo.center = CGPointMake(self.view.center.x, 150);
    [self.view addSubview:levelInfo];
    
    // Game status
    UILabel *statusLabel = [[UILabel alloc] init];
    statusLabel.text = @"Game Running";
    statusLabel.textColor = [UIColor greenColor];
    statusLabel.font = [UIFont systemFontOfSize:16];
    statusLabel.textAlignment = NSTextAlignmentCenter;
    statusLabel.frame = CGRectMake(0, 0, 200, 30);
    statusLabel.center = CGPointMake(self.view.center.x, 200);
    [self.view addSubview:statusLabel];
    
    // Close button
    UIButton *closeButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [closeButton setTitle:@"Close Game" forState:UIControlStateNormal];
    [closeButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    closeButton.backgroundColor = [UIColor colorWithRed:0.8 green:0.2 blue:0.2 alpha:1.0];
    closeButton.layer.cornerRadius = 8;
    closeButton.frame = CGRectMake(0, 0, 120, 44);
    closeButton.center = CGPointMake(self.view.center.x, self.view.bounds.size.height - 100);
    [closeButton addTarget:self action:@selector(closeGame) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:closeButton];
    
    // Add some game-like elements
    [self addGameElements];
}

- (void)addGameElements {
    // Add some visual elements to make it look more like a game
    for (int i = 0; i < 5; i++) {
        UIView *element = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 20, 20)];
        element.backgroundColor = [UIColor colorWithRed:0.2 green:0.6 blue:1.0 alpha:0.7];
        element.layer.cornerRadius = 10;
        element.center = CGPointMake(50 + i * 60, 300 + (i % 2) * 40);
        [self.view addSubview:element];
        
        // Animate the elements
        [UIView animateWithDuration:2.0 delay:i * 0.2 options:UIViewAnimationOptionRepeat | UIViewAnimationOptionAutoreverse animations:^{
            element.alpha = 0.3;
            element.transform = CGAffineTransformMakeScale(1.2, 1.2);
        } completion:nil];
    }
}

- (void)closeGame {
    [self dismissViewControllerAnimated:YES completion:nil];
}

@end 