import Firebase

import PromiseKit

extension Firebase {
  func observeSingleEventOfType(eventType: FEventType) -> Promise<FDataSnapshot> {
    return Promise<FDataSnapshot>() { fulfill, reject in
      observeSingleEventOfType(eventType, withBlock: { snapshot in
        fulfill(snapshot)
      })
    }
  }

  func authWithOAuthProvider(provider: String, token: String) -> Promise<FAuthData> {
    return Promise<FAuthData> { fulfill, reject in
      authWithOAuthProvider(provider, token: token, withCompletionBlock: { (error, authData) in
        if let err = error {
          reject(err)
        } else {
          fulfill(authData)
        }
      })
    }
  }
}

class GoogleSignIn : GIDSignInDelegate {
  enum SignInStatus {
    case Yes
    case No
    case Maybe
  }

  var signInStatus : SignInStatus = .Maybe
  var signInPromise : (promise: Promise<GIDGoogleUser>, fulfill: (GIDGoogleUser)->Void, reject: (ErrorType)->Void)?
  var signedInUser : GIDGoogleUser?

  required init() {
    var configureError:NSError?
    GGLContext.sharedInstance().configureWithError(&configureError)
    assert(configureError == nil, "Error configuring Google services: \(configureError)")
    GIDSignIn.sharedInstance().delegate = self
  }

  func signIn(delegate: GIDSignInUIDelegate!) -> Promise<GIDGoogleUser> {
    if let triple = signInPromise {
      assert(signInStatus != .Yes)
      return triple.promise
    } else if let user = signedInUser {
      assert(signInStatus == .Yes)
      return Promise<GIDGoogleUser> { fulfill, reject in fulfill(user) }
    } else {
      assert(signInStatus != .Yes)
      assert(GIDSignIn.sharedInstance().uiDelegate == nil)
      GIDSignIn.sharedInstance().uiDelegate = delegate

      let triple = Promise<GIDGoogleUser>.pendingPromise()
      signInPromise = triple
      if signInStatus == .No {
        GIDSignIn.sharedInstance().signIn()
      } else {
        GIDSignIn.sharedInstance().signInSilently()
      }
      return triple.promise
    }
  }

  func signOut() {
    if let triple = signInPromise {
      assert(signInStatus != .Yes, "Promise should have been cleared")
      signInPromise = nil
      signInStatus = .No
      GIDSignIn.sharedInstance().signOut()

      // TODO: sensible cancellation code
      triple.reject(NSError(domain: "Qi", code: 123, userInfo: nil))

      // TODO: provide way for auth-users to be notified of un-auth.
      //firebase.unauth()
    } else if (signInStatus == .Yes) {
      assert(signInPromise == nil, "Should already have been cleared")
      signInStatus = .No
      signedInUser = nil

      GIDSignIn.sharedInstance().signOut()

      // TODO: provide way for auth-users to be notified of un-auth.
      //firebase.unauth()
    }
  }


  // GIDSignInDelegate method.  Sign into Firebase upon successful login.
  @objc func signIn(signIn: GIDSignIn!, didSignInForUser user: GIDGoogleUser!,
    withError error: NSError!) {
      if (error == nil) {
        signInStatus = .Yes
        signedInUser = user
        GIDSignIn.sharedInstance().uiDelegate = nil
        let triple = signInPromise!
        signInPromise = nil
        triple.fulfill(user)
      } else {
        if (signInStatus == .No) {
          // Non-silent sign-in failed.  Give up.
          // (don't assert this error; it is commonly returned as nil)
          print("SkeqiFirebase failed Google sign-in: \(error.localizedDescription)")
        } else {
          // Silent sign-in failed.  Attempt sign-in with user interaction.
          signInStatus = .No
          GIDSignIn.sharedInstance().signIn()
        }
      }
  }

  // GIDSignInDelegate method.  Unauth Firebase when disconnected from Google.
  @objc func signIn(signIn: GIDSignIn!, didDisconnectWithUser user:GIDGoogleUser!,
    withError error: NSError!) {
      // TODO: I think this should work fine (e.g. shouldn't inifinite-loop), but needs testing.
      signOut()
  }


}

class SkeqiFirebase {

  private let googleSignIn = GoogleSignIn()

  let firebase : Firebase

  private var firebaseAuthData : FAuthData?
  private let handles = []

  var userName : String = ""
  var email : String = ""
  var uid : String = ""

  required init(url: String!) {
    firebase = Firebase(url: url)

    firebase.observeAuthEventWithBlock({ authData in
      print("AUTH DATA CHANGED: \(authData)")
/*
      if authData == nil {
        assert(self.firebaseAuthData == nil, "TODO: handle deauth")
        // No user is signed in
        self.firebaseAuthData = nil
      } else {
        if let firebaseAuthData = self.firebaseAuthData {
          assert(firebaseAuthData.uid == authData.uid, "TODO: handle account changes");
        }

        self.firebaseAuthData = authData

        let future = self.firebase.observeSingleEventOfType(.Value)
        future.onSuccess


        self.firebase.observeSingleEventOfType(
          .Value,
          withBlock: { snapshot in
            if snapshot.exists() {

            } else {
              self.firebase.childByAppendingPath("users/\(authData.uid)").setValue([
                "name": name,
                "email": email,
                "pages": [],
                "currentPage": "0"])
            }
          })
      }*/
    })
  }

  func signIn(delegate delegate: GIDSignInUIDelegate!) -> Promise<FAuthData> {
    return googleSignIn.signIn(delegate).then { user in
      let triple = Promise<FAuthData>.pendingPromise()
      let token = user.authentication.accessToken
      self.firebase.authWithOAuthProvider("google", token: token, withCompletionBlock: { (error, authData) in
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
      return triple.promise
    }
  }
}


class SkeqiFirebasePage {

}
