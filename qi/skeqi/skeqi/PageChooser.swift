//
//  PageChooser.swift
//  skeqi
//
//  Created by Josh Gargus on 4/23/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

/*

import UIKit



class PageChooser: CollectionViewController {
  let ref = Firebase(url: "https://blistering-inferno-9169.firebaseio.com");
  
  override func viewDidLoad() {
    super.viewDidLoad()
    
    
    // Set up swipe to delete
    tableView.allowsMultipleSelectionDuringEditing = false
    
    // User Count
    userCountBarButtonItem = UIBarButtonItem(title: "1", style: UIBarButtonItemStyle.Plain, target: self, action: #selector(userCountButtonDidTouch))
    userCountBarButtonItem.tintColor = UIColor.whiteColor()
    navigationItem.leftBarButtonItem = userCountBarButtonItem
    
    user = User(uid: "FakeId", email: "hungry@person.food")
  }
  
  override func viewDidAppear(animated: Bool) {
    super.viewDidAppear(animated)
    ref.observeAuthEventWithBlock { authData in
      if authData != nil {
        self.user = User(authData: authData)
        
        
        
        let groceryListRef =
          self.ref.childByAppendingPath("users/\(self.user.uid)/groceryList")
        groceryListRef.observeEventType(.Value, withBlock: { snapshot in
          print("OBSERVED VALUE \(snapshot.value)")
          
          var newItems = [GroceryItem]()
          for item in snapshot.children {
            let groceryItem = GroceryItem(firebaseSnapshot: item as! FDataSnapshot)
            newItems.append(groceryItem)
          }
          self.items = newItems
          self.tableView.reloadData()
          
          }, withCancelBlock: { error in
            print("OBSERVATION ERROR: \(error.description)")
            self.ref.unauth()
        })
      }
    }
  }
  
}
*/