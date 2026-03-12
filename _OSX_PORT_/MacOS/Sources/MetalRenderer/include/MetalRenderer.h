#pragma once
#import <AppKit/AppKit.h>

// WaterRace renderer.
// mode: nil = logo intro only; @"characters" / @"boats" / @"levels" = viewer.
@interface LogoIntroView : NSView
- (instancetype)initWithFrame:(NSRect)frame mode:(nullable NSString *)mode;
@end
