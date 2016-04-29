//
//  QiPlusPlus.cpp
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

#include <stdio.h>

#import "QiPlusPlus.h"

#include "qi/qi.h"
#include "qi/dbg/server.h"


@interface QiPlusPlus ()

@end

@implementation QiPlusPlus

- (void)start {
  qi::Qi::Init();
}

- (void)shutdown {
  qi::Qi::Shutdown();
}

- (void)startDebugServer {
  qi::dbg::Server::Start();
}

- (void)stopDebugServer {
  qi::dbg::Server::Stop();
}

@end
