//
//  LoginViewController.swift
//  skeqi
//
//  Created by Josh Gargus on 4/25/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import UIKit
import FirebaseAuth
import FirebaseDatabase
import PromiseKit

class LoginViewController : UIViewController, GIDSignInUIDelegate {
  let loginSegue = "Login"
  let firebaseProvider = UIApplication.sharedApplication().delegate as! FirebaseRefProvider
  let ref = FIRDatabase.database().reference()
  let auth = FIRAuth.auth()!
  
  override func viewDidAppear(animated: Bool) {
    super.viewDidAppear(animated)
    
    firebaseProvider.googleSignIn.signIn(self).then { (user) -> Void in
      let triple = Promise<FIRUser>.pendingPromise()
      let authentication = user.authentication
      let credential = FIRGoogleAuthProvider.credentialWithIDToken(authentication.idToken,
        accessToken: authentication.accessToken)
      
      FIRAuth.auth()!.signInWithCredential(credential) { (user, error) in
        if let err = error {
          print("OAuth failed")
          triple.reject(err)
        } else {
          // TODO: also return "auth lost" future that clients can wait on
          // TODO: what to do about multiple provider data?
          let providerData = user!.providerData[0]
          let name: String = providerData.displayName!
          let email: String = providerData.email!
          print("=-=-=-=-=-= Firebase sign-in successful for \(name) (\(email)).")
          triple.fulfill(user!)
        }
      }
    }
    
    // TODO: handle failures better
    var handle : FIRAuthStateDidChangeListenerHandle?
    handle = auth.addAuthStateDidChangeListener { authData in
      if let user = authData.1 {
        self.auth.removeAuthStateDidChangeListener(handle!)
        let userRef = self.ref.child("users/\(user.uid)")
        self.firebaseProvider.setFirebaseRef(userRef)
        self.performSegueWithIdentifier(self.loginSegue, sender: nil)
      }
    }
  }
}
