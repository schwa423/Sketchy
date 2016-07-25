This project uses git submodules, so you'll need to do the normal thing:
```sh
git submodule init
git submodule update
```

Similarly, third-party code is brought in by Cocoapods, and must be initialized:
```sh
pod install
```

Finally, you'll need to bring your own GoogleService-Info.plist: follow the instructions in the "Add Firebase to your app" 
section of the [Firebase iOS setup instructions](https://firebase.google.com/docs/ios/setup).
