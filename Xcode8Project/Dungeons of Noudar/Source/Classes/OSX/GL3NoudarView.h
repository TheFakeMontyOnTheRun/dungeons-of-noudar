//
//  GL3NoudarView.h
//  Dungeons of Noudar
//
//  Created by Daniel Monteiro on 06/11/16.
//  Copyright © 2016 Ostrich Design Bureau. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>

#import "modelUtil.h"
#import "imageUtil.h"


@interface GL3NoudarView : NSOpenGLView {
	CVDisplayLinkRef displayLink;
}
@end
