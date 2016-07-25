//
//  AppDelegate.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import UIKit
import FirebaseDatabase

protocol FirebaseRefProvider {
  func getFirebaseRef() -> FIRDatabaseReference
  func setFirebaseRef(firebase: FIRDatabaseReference?)
  var pageId: String { get set }
  
  var googleSignIn: GoogleSignIn { get }
}

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate, FirebaseRefProvider {
  var window: UIWindow?
  let qi = QiPlusPlus();
  
  var firebase : FIRDatabaseReference?
  func getFirebaseRef() -> FIRDatabaseReference { return firebase! }
  func setFirebaseRef(firebase: FIRDatabaseReference?) { self.firebase = firebase }
  var pageId: String = "0"
  let googleSignIn = GoogleSignIn()
  
  override init() {
    super.init()
  }
  
  func application(app: UIApplication, willFinishLaunchingWithOptions options: [NSObject : AnyObject]?) -> Bool {
    qi.start()
    return true
  }
  
  func application(application: UIApplication, didFinishLaunchingWithOptions launchOptions: [NSObject : AnyObject]?) -> Bool {
    FIRApp.configure()
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

  // TODO: this method is deprecated.  The new hotness is application(openURL:options:).
  func application(app: UIApplication, openURL url: NSURL, sourceApplication sourceApp: String?, annotation: AnyObject) -> Bool {
    return GIDSignIn.sharedInstance().handleURL(url, sourceApplication: sourceApp, annotation: annotation);
  }
}
