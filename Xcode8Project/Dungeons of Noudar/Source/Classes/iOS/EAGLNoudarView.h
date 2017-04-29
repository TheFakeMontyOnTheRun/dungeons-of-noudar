//
//  EAGLNoudarView.h
//  Dungeons of Noudar
//
//  Created by Daniel Monteiro on 06/11/16.
//  Copyright © 2016 Ostrich Design Bureau. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface EAGLNoudarView : UIView {
	UISwipeGestureRecognizer* mSwipeLeftRecognizer;
	UISwipeGestureRecognizer* mSwipeRightRecognizer;
	UISwipeGestureRecognizer* mSwipeUpRecognizer;
	UISwipeGestureRecognizer* mSwipeDownRecognizer;
	
	UISwipeGestureRecognizer* mDoubleSwipeLeftRecognizer;
	UISwipeGestureRecognizer* mDoubleSwipeRightRecognizer;
	
	UITapGestureRecognizer* mDoubleTapRecognizer;
	UILongPressGestureRecognizer* mLongTapRecognizer;
}

- (void) drawView:(id)sender;
-(void) onSwipeLeft:(UISwipeGestureRecognizer *)recognizer;
-(void) onSwipeRight:(UISwipeGestureRecognizer *)recognizer;
-(void) onSwipeUp;
-(void) onSwipeDown;
-(void) onDoubleTap;
-(void) onLongTap;

@end
