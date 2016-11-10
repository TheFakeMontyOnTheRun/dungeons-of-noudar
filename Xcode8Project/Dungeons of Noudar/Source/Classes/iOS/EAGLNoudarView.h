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
}







//	@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
//	@property (nonatomic) NSInteger animationFrameInterval;

//	- (void) startAnimation;
//	- (void) stopAnimation;



	- (void) drawView:(id)sender;

-(void) onSwipeLeft;
-(void) onSwipeRight;
-(void) onSwipeUp;
-(void) onSwipeDown;
@end
