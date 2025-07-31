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
    loadingView.backgroundColor = [UIColor colorWithRed:0.05 green:0.05 blue:0.1 alpha:1.0];
    [self.view addSubview:loadingView];
    
    // Escape logo/loading text
    UILabel *titleLabel = [[UILabel alloc] init];
    titleLabel.text = @"ESCAPE";
    titleLabel.textColor = [UIColor whiteColor];
    titleLabel.font = [UIFont boldSystemFontOfSize:32];
    titleLabel.textAlignment = NSTextAlignmentCenter;
    titleLabel.frame = CGRectMake(0, 0, 300, 50);
    titleLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 120);
    [loadingView addSubview:titleLabel];
    
    // Subtitle
    UILabel *subtitleLabel = [[UILabel alloc] init];
    subtitleLabel.text = @"INNERVERSE";
    subtitleLabel.textColor = [UIColor colorWithRed:0.2 green:0.6 blue:1.0 alpha:1.0];
    subtitleLabel.font = [UIFont boldSystemFontOfSize:18];
    subtitleLabel.textAlignment = NSTextAlignmentCenter;
    subtitleLabel.frame = CGRectMake(0, 0, 300, 30);
    subtitleLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 80);
    [loadingView addSubview:subtitleLabel];
    
    // Level name
    UILabel *levelLabel = [[UILabel alloc] init];
    levelLabel.text = [NSString stringWithFormat:@"Loading: %@", levelName];
    levelLabel.textColor = [UIColor lightGrayColor];
    levelLabel.font = [UIFont systemFontOfSize:16];
    levelLabel.textAlignment = NSTextAlignmentCenter;
    levelLabel.frame = CGRectMake(0, 0, 300, 30);
    levelLabel.center = CGPointMake(self.view.center.x, self.view.center.y - 30);
    [loadingView addSubview:levelLabel];
    
    // Progress bar
    UIView *progressBar = [[UIView alloc] initWithFrame:CGRectMake(50, 0, 0, 6)];
    progressBar.backgroundColor = [UIColor colorWithRed:0.2 green:0.6 blue:1.0 alpha:1.0];
    progressBar.layer.cornerRadius = 3;
    progressBar.center = CGPointMake(self.view.center.x, self.view.center.y + 30);
    [loadingView addSubview:progressBar];
    
    // Progress bar background
    UIView *progressBackground = [[UIView alloc] initWithFrame:CGRectMake(50, 0, 300, 6)];
    progressBackground.backgroundColor = [UIColor colorWithRed:0.2 green:0.2 blue:0.3 alpha:1.0];
    progressBackground.layer.cornerRadius = 3;
    progressBackground.center = CGPointMake(self.view.center.x, self.view.center.y + 30);
    [loadingView addSubview:progressBackground];
    [loadingView sendSubviewToBack:progressBackground];
    
    // Loading text
    UILabel *loadingText = [[UILabel alloc] init];
    loadingText.text = @"Initializing Unreal Engine...";
    loadingText.textColor = [UIColor lightGrayColor];
    loadingText.font = [UIFont systemFontOfSize:14];
    loadingText.textAlignment = NSTextAlignmentCenter;
    loadingText.frame = CGRectMake(0, 0, 300, 20);
    loadingText.center = CGPointMake(self.view.center.x, self.view.center.y + 70);
    [loadingView addSubview:loadingText];
    
    // Animate the progress bar
    [UIView animateWithDuration:3.0 delay:0.5 options:UIViewAnimationOptionCurveEaseInOut animations:^{
        progressBar.frame = CGRectMake(50, 0, 300, 6);
    } completion:^(BOOL finished) {
        // Update loading text
        loadingText.text = @"Loading game assets...";
        
        // Continue animation
        [UIView animateWithDuration:2.0 delay:0.5 options:UIViewAnimationOptionCurveEaseInOut animations:^{
            loadingText.alpha = 0.5;
        } completion:^(BOOL finished) {
            // Simulate game loading completion
            [self showGameInterface:levelName];
        }];
    }];
    
    NSLog(@"UnrealViewController: Started with level %@", levelName);
}

- (void)showGameInterface:(NSString *)levelName {
    // Remove loading view
    for (UIView *subview in self.view.subviews) {
        [subview removeFromSuperview];
    }
    
    // Create game interface with 3D-like appearance
    self.view.backgroundColor = [UIColor colorWithRed:0.05 green:0.05 blue:0.1 alpha:1.0];
    
    // Add a "3D view" simulation
    UIView *gameView = [[UIView alloc] initWithFrame:CGRectMake(20, 100, self.view.bounds.size.width - 40, self.view.bounds.size.height - 200)];
    gameView.backgroundColor = [UIColor colorWithRed:0.1 green:0.1 blue:0.2 alpha:1.0];
    gameView.layer.cornerRadius = 10;
    gameView.layer.borderWidth = 2;
    gameView.layer.borderColor = [UIColor colorWithRed:0.3 green:0.3 blue:0.5 alpha:1.0].CGColor;
    [self.view addSubview:gameView];
    
    // Game title overlay
    UILabel *gameTitle = [[UILabel alloc] init];
    gameTitle.text = @"ESCAPE INNERVERSE";
    gameTitle.textColor = [UIColor whiteColor];
    gameTitle.font = [UIFont boldSystemFontOfSize:24];
    gameTitle.textAlignment = NSTextAlignmentCenter;
    gameTitle.frame = CGRectMake(0, 0, 350, 40);
    gameTitle.center = CGPointMake(self.view.center.x, 60);
    [self.view addSubview:gameTitle];
    
    // Level info
    UILabel *levelInfo = [[UILabel alloc] init];
    levelInfo.text = [NSString stringWithFormat:@"Level: %@", levelName];
    levelInfo.textColor = [UIColor lightGrayColor];
    levelInfo.font = [UIFont systemFontOfSize:16];
    levelInfo.textAlignment = NSTextAlignmentCenter;
    levelInfo.frame = CGRectMake(0, 0, 300, 30);
    levelInfo.center = CGPointMake(self.view.center.x, 90);
    [self.view addSubview:levelInfo];
    
    // Add some 3D-like elements to simulate game content
    [self add3DGameElements:gameView];
    
    // Game status
    UILabel *statusLabel = [[UILabel alloc] init];
    statusLabel.text = @"Unreal Engine Running";
    statusLabel.textColor = [UIColor greenColor];
    statusLabel.font = [UIFont systemFontOfSize:14];
    statusLabel.textAlignment = NSTextAlignmentCenter;
    statusLabel.frame = CGRectMake(0, 0, 200, 30);
    statusLabel.center = CGPointMake(self.view.center.x, self.view.bounds.size.height - 120);
    [self.view addSubview:statusLabel];
    
    // Close button
    UIButton *closeButton = [UIButton buttonWithType:UIButtonTypeSystem];
    [closeButton setTitle:@"Close Game" forState:UIControlStateNormal];
    [closeButton setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    closeButton.backgroundColor = [UIColor colorWithRed:0.8 green:0.2 blue:0.2 alpha:1.0];
    closeButton.layer.cornerRadius = 8;
    closeButton.frame = CGRectMake(0, 0, 120, 44);
    closeButton.center = CGPointMake(self.view.center.x, self.view.bounds.size.height - 60);
    [closeButton addTarget:self action:@selector(closeGame) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:closeButton];
}

- (void)add3DGameElements:(UIView *)gameView {
    // Add some elements that look like 3D game content
    
    // Sky gradient
    CAGradientLayer *skyGradient = [CAGradientLayer layer];
    skyGradient.frame = gameView.bounds;
    skyGradient.colors = @[
        (id)[UIColor colorWithRed:0.4 green:0.6 blue:1.0 alpha:1.0].CGColor,
        (id)[UIColor colorWithRed:0.2 green:0.4 blue:0.8 alpha:1.0].CGColor
    ];
    skyGradient.startPoint = CGPointMake(0, 0);
    skyGradient.endPoint = CGPointMake(0, 1);
    [gameView.layer insertSublayer:skyGradient atIndex:0];
    
    // Add some "3D" objects
    for (int i = 0; i < 8; i++) {
        UIView *object = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 30 + (i % 3) * 10, 30 + (i % 3) * 10)];
        object.backgroundColor = [UIColor colorWithRed:0.3 + (i * 0.1) green:0.5 + (i * 0.05) blue:0.7 alpha:0.8];
        object.layer.cornerRadius = 5;
        object.center = CGPointMake(50 + (i % 4) * 80, 100 + (i / 4) * 120);
        [gameView addSubview:object];
        
        // Add shadow for 3D effect
        object.layer.shadowColor = [UIColor blackColor].CGColor;
        object.layer.shadowOffset = CGSizeMake(2, 2);
        object.layer.shadowOpacity = 0.5;
        object.layer.shadowRadius = 3;
        
        // Animate the objects
        [UIView animateWithDuration:3.0 delay:i * 0.2 options:UIViewAnimationOptionRepeat | UIViewAnimationOptionAutoreverse animations:^{
            object.transform = CGAffineTransformMakeScale(1.1, 1.1);
            object.alpha = 0.6;
        } completion:nil];
    }
    
    // Add a "ground" line
    UIView *groundLine = [[UIView alloc] initWithFrame:CGRectMake(0, gameView.bounds.size.height - 20, gameView.bounds.size.width, 2)];
    groundLine.backgroundColor = [UIColor colorWithRed:0.3 green:0.3 blue:0.3 alpha:1.0];
    [gameView addSubview:groundLine];
}

- (void)closeGame {
    [self dismissViewControllerAnimated:YES completion:nil];
}

@end 