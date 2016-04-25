//
//  LoginViewController.swift
//  skeqi
//
//  Created by Josh Gargus on 4/25/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import UIKit
import Firebase
import PromiseKit

class LoginViewController : UIViewController, GIDSignInUIDelegate {
  let loginSegue = "Login"
  let firebaseProvider = UIApplication.sharedApplication().delegate as! FirebaseRefProvider
  let ref = Firebase(url: "https://blistering-inferno-9169.firebaseio.com");
  
  override func viewDidAppear(animated: Bool) {
    super.viewDidAppear(animated)
    
    firebaseProvider.googleSignIn.signIn(self).then { (user) -> Void in
      let triple = Promise<FAuthData>.pendingPromise()
      let token = user.authentication.accessToken
      self.ref.authWithOAuthProvider("google", token: token, withCompletionBlock: { (error, authData) in
        if let err = error {
          print("OAuth failed")
          triple.reject(err)
        } else {
          // TODO: also return "auth lost" future that clients can wait on
          let providerData = authData.providerData as! Dictionary<String, AnyObject>
          let name: String = providerData["displayName"] as! String;
          let email: String = providerData["email"] as! String;
          print("=-=-=-=-=-= Firebase sign-in successful for \(name) (\(email)).")
          triple.fulfill(authData)
        }
      })
    }
    
    // TODO: handle failures better
    var handle : UInt = 0;
    handle = ref.observeAuthEventWithBlock { (authData) -> Void in
      if authData != nil {
        self.ref.removeAuthEventObserverWithHandle(handle)

        let userRef = self.ref.childByAppendingPath("users/\(authData.uid)")
        self.firebaseProvider.setFirebaseRef(userRef)
        self.performSegueWithIdentifier(self.loginSegue, sender: nil)
      }
    }
  }
}
