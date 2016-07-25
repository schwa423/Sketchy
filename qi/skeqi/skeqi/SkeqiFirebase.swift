import FirebaseAuth
import FirebaseDatabase

import PromiseKit

extension FIRDatabaseReference {
  func observeSingleEventOfType(eventType: FIRDataEventType) -> Promise<FIRDataSnapshot> {
    return Promise<FIRDataSnapshot>() { fulfill, reject in
      self.observeSingleEventOfType(eventType, withBlock: { snapshot in
        fulfill(snapshot)
      })
    }
  }
}

class GoogleSignIn : NSObject, GIDSignInDelegate {
  enum SignInStatus {
    case Yes
    case No
    case Maybe
  }

  var signInStatus : SignInStatus = .Maybe
  var signInPromise : (promise: Promise<GIDGoogleUser>, fulfill: (GIDGoogleUser)->Void, reject: (ErrorType)->Void)?
  var signedInUser : GIDGoogleUser?

  override required init() {
    super.init()
    var configureError:NSError?
    GGLContext.sharedInstance().configureWithError(&configureError)
    assert(configureError == nil, "Error configuring Google services: \(configureError)")
    GIDSignIn.sharedInstance().clientID = FIRApp.defaultApp()?.options.clientID
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

  let firebase : FIRDatabaseReference

  required init(url: String!) {
    firebase = FIRDatabase.database().reference()
    
    FIRAuth.auth()!.addAuthStateDidChangeListener({ authData in
      // TODO: handle auth changes
      print("AUTH DATA CHANGED: \(authData)")
    })
  }

  func signIn(delegate delegate: GIDSignInUIDelegate!) -> Promise<FIRUser> {
    return googleSignIn.signIn(delegate).then { user in
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
    
      return triple.promise
    }
  }
}
