import Firebase

//import GoogleSignIn

import BrightFutures
import Result

extension Firebase {
  func observeSingleEventOfType(eventType: FEventType) -> Future<FDataSnapshot, BrightFutures.NoError> {
    let promise = Promise<FDataSnapshot, BrightFutures.NoError>()
    observeSingleEventOfType(eventType, withBlock: { snapshot in
      promise.success(snapshot)
    })
    return promise.future
  }

  func authWithOAuthProvider(provider: String, token: String) -> Future<FAuthData, NSError> {
    let promise = Promise<FAuthData, NSError>()
    authWithOAuthProvider(provider, token: token, withCompletionBlock: { (error, authData) in
      if let error = error {
        promise.failure(error)
      } else {
        promise.success(authData)
      }
    })
    return promise.future
  }
}

class GoogleSignIn : GIDSignInDelegate {
  enum SignInStatus {
    case Yes
    case No
    case Maybe
  }

  var signInStatus : SignInStatus = .Maybe
  var signInPromise : Promise<GIDGoogleUser, NSError>?
  var signedInUser : GIDGoogleUser?

  required init() {
    var configureError:NSError?
    GGLContext.sharedInstance().configureWithError(&configureError)
    assert(configureError == nil, "Error configuring Google services: \(configureError)")
    GIDSignIn.sharedInstance().delegate = self
  }

  func signIn(delegate: GIDSignInUIDelegate!) -> Future<GIDGoogleUser, NSError> {
    if let signInPromise = signInPromise {
      assert(signInStatus != .Yes)
      return signInPromise.future
    } else if let signedInUser = signedInUser {
      assert(signInStatus == .Yes)
      let promise = Promise<GIDGoogleUser, NSError>()
      promise.success(signedInUser)
      return promise.future
    } else {
      assert(signInStatus != .Yes)
      assert(GIDSignIn.sharedInstance().uiDelegate == nil)
      GIDSignIn.sharedInstance().uiDelegate = delegate

      let promise = Promise<GIDGoogleUser, NSError>()
      signInPromise = promise
      if signInStatus == .No {
        GIDSignIn.sharedInstance().signIn()
      } else {
        GIDSignIn.sharedInstance().signInSilently()
      }
      return promise.future
    }
  }

  func signOut() {
    if let promise = signInPromise {
      assert(signInStatus != .Yes, "Promise should have been cleared")
      signInPromise = nil
      signInStatus = .No
      GIDSignIn.sharedInstance().signOut()

      // TODO: sensible cancellation code
      promise.failure(NSError(domain: "Qi", code: 123, userInfo: nil))

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
        let promise = signInPromise!
        signInPromise = nil
        promise.success(user)
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

  private let firebase : Firebase

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

  func signIn(delegate delegate: GIDSignInUIDelegate!) -> Future<FAuthData, NSError> {
    let promise = Promise<FAuthData, NSError>()

    googleSignIn.signIn(delegate)
      .onSuccess { user in
        let token = user.authentication.accessToken
        self.firebase.authWithOAuthProvider("google", token: token, withCompletionBlock: { (error, authData) in
          if let error = error {
            print("OAuth failed")
            promise.failure(error)
          } else {
            // TODO: also return "auth lost" future that clients can wait on
            let providerData = authData.providerData as! Dictionary<String, AnyObject>
            let name: String = providerData["displayName"] as! String;
            let email: String = providerData["email"] as! String;
            print("=-=-=-=-=-= Firebase sign-in successful for \(name) (\(email)).")
            promise.success(authData)
          }
        })
      }
      .onFailure { error in
        print("Failed to obtain Google login token: \(error)")
        promise.failure(error)
      }

    return promise.future
  }
}

class SkeqiFirebasePage {

}
