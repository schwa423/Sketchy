//
//  QiPlusPlus.h
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

#ifndef QiPlusPlus_h
#define QiPlusPlus_h

#import <Foundation/Foundation.h>

@interface QiPlusPlus : NSObject
- (void)start;
- (void)shutdown;
- (void)startDebugServer;
- (void)stopDebugServer;
@end

#endif /* QiPlusPlus_h */
