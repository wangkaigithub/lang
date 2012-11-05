    //
//  CustomPickerViewController.m
//  07-Pickers
//
//  Created by nuoerlz on 12-6-17.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "CustomPickerViewController.h"
#import <AudioToolbox/AudioToolbox.h>


@implementation CustomPickerViewController

@synthesize picker;
@synthesize winLabel;
@synthesize column1;
@synthesize column2;
@synthesize column3;
@synthesize column4;
@synthesize column5;
@synthesize button;

-(void)showButton
{
    button.hidden = NO;
}

-(void)playWinSound
{
    NSString *path = [[NSBundle mainBundle] pathForResource:@"win" 
                                                     ofType:@"wav"];
    SystemSoundID soundID;
    AudioServicesCreateSystemSoundID((CFURLRef)[NSURL fileURLWithPath:path]
                                     , &soundID);
    AudioServicesPlaySystemSound (soundID);
    winLabel.text = @"WIN!";
    [self performSelector:@selector(showButton) withObject:nil 
               afterDelay:1.5];
}

-(IBAction)spin:(id)sender
{
    BOOL win = NO;
    int numInRow = 1;
    int lastVal = -1;
    for (int i = 0; i < 5; i++)
    {
        int newValue = random() % [self.column1 count];
        
        if (newValue == lastVal)
            numInRow++;
        else
            numInRow = 1;
        
        lastVal = newValue;
        [picker selectRow:newValue inComponent:i animated:YES];
        [picker reloadComponent:i];
        if (numInRow >= 3)
            win = YES;
    }
    button.hidden = YES;
    NSString *path = [[NSBundle mainBundle] pathForResource:@"crunch" 
                                                     ofType:@"wav"];
    SystemSoundID soundID;
    AudioServicesCreateSystemSoundID((CFURLRef)[NSURL fileURLWithPath:path]
                                     , &soundID);
    AudioServicesPlaySystemSound (soundID);
    
    if (win)
        [self performSelector:@selector(playWinSound) 
                   withObject:nil 
                   afterDelay:1.5]; // 时间延迟不起作用？QQQQQ
    else
        [self performSelector:@selector(showButton) 
                   withObject:nil 
                   afterDelay:1.5];
    
    winLabel.text = @"";
}

- (void)viewDidLoad {
    
    UIImage *seven = [UIImage imageNamed:@"seven.png"];
    UIImage *bar = [UIImage imageNamed:@"bar.png"];
    UIImage *crown = [UIImage imageNamed:@"crown.png"];
    UIImage *cherry = [UIImage imageNamed:@"cherry.png"];
    UIImage *lemon = [UIImage imageNamed:@"lemon.png"];
    UIImage *apple = [UIImage imageNamed:@"apple.png"];
    
    for (int i = 1; i <= 5; i++)
    {
        UIImageView *sevenView = [[UIImageView alloc] initWithImage:seven];
        UIImageView *barView = [[UIImageView alloc] initWithImage:bar];
        UIImageView *crownView = [[UIImageView alloc] initWithImage:crown];
        UIImageView *cherryView = [[UIImageView alloc] 
                                   initWithImage:cherry];
        UIImageView *lemonView = [[UIImageView alloc] initWithImage:lemon];
        UIImageView *appleView = [[UIImageView alloc] initWithImage:apple];
        
        NSArray *imageViewArray = [[NSArray alloc] initWithObjects: 
                                   sevenView, barView, crownView, cherryView, lemonView, 
                                   appleView, nil];
        
        NSString *fieldName = 
        [[NSString alloc] initWithFormat:@"column%d", i];
        [self setValue:imageViewArray forKey:fieldName]; // 初始化
        [fieldName release];
        [imageViewArray release];
        
        [sevenView release];
        [barView release];
        [crownView release];
        [cherryView release];
        [lemonView release];
        [appleView release];
    }
    
    srandom(time(NULL));
}


// The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
/*
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization.
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/

/*
// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}
*/

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload {
	self.picker = nil;
    self.winLabel = nil;
    self.column1 = nil;
    self.column2 = nil;
    self.column3 = nil;
    self.column4 = nil;
    self.column5 = nil;
    self.button = nil;
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [picker release];
    [winLabel release];
    [column1 release];
    [column2 release];
    [column3 release];
    [column4 release];
    [column5 release];
    [button release];
    [super dealloc];
}


#pragma mark -
#pragma mark Picker Data Source Methods

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 5;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView 
numberOfRowsInComponent:(NSInteger)component
{
    return [self.column1 count];
}

#pragma mark Picker Delegate Methods

// 该委托方法返回的是view视图。
- (UIView *)pickerView:(UIPickerView *)pickerView 
            viewForRow:(NSInteger)row
          forComponent:(NSInteger)component reusingView:(UIView *)view
{
    NSString *arrayName = [[NSString alloc] initWithFormat:@"column%d", 
                           component+1];
    NSArray *array = [self valueForKey:arrayName];
    [arrayName release];
    return [array objectAtIndex:row];
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

@end
