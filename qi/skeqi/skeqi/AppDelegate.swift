//
//  AppDelegate.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import UIKit
import Firebase

protocol FirebaseRefProvider {
  func getFirebaseRef() -> Firebase
  func setFirebaseRef(firebase: Firebase?)
  var pageId: String { get set }
  
  var googleSignIn: GoogleSignIn { get }
}

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate, FirebaseRefProvider {
  var window: UIWindow?
  let qi = QiPlusPlus();
  
  var firebase : Firebase?
  func getFirebaseRef() -> Firebase { return firebase! }
  func setFirebaseRef(firebase: Firebase?) { self.firebase = firebase }
  var pageId: String = "0"
  let googleSignIn = GoogleSignIn()
  
  override init() {
    super.init()
  }
  
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
