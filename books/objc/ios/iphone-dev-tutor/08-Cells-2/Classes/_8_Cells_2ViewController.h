//
//  _8_Cells_2ViewController.h
//  08-Cells-2
//
//  Created by nuoerlz on 12-6-18.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#define    kTableViewRowHeight    (65 + 1)

@interface _8_Cells_2ViewController : UIViewController 
		<UITableViewDataSource, UITableViewDelegate>
{
    NSArray    *computers;
}

@property (nonatomic, retain) NSArray *computers;


@end

