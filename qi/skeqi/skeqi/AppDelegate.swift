//
//  AppDelegate.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {
  var window: UIWindow?
  let qi = QiPlusPlus();
  
  func application(app: UIApplication, willFinishLaunchingWithOptions options: [NSObject : AnyObject]?) -> Bool {
    qi.start()
    return true
  }
  
  func applicationWillTerminate(app: UIApplication) {
    qi.shutdown()
  }
  
  func applicationDidBecomeActive(app: UIApplication) {
    qi.startDebugServer()
  }
  
  func applicationWillResignActive(app: UIApplication) {
    qi.stopDebugServer()
  }

  func application(app: UIApplication, openURL url: NSURL, sourceApplication sourceApp: String?, annotation: AnyObject) -> Bool {
    return GIDSignIn.sharedInstance().handleURL(url, sourceApplication: sourceApp, annotation: annotation);
  }
}
