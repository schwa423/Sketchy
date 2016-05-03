//
//  PageChooser.swift
//  skeqi
//
//  Created by Josh Gargus on 4/23/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import UIKit
import Firebase

class PageCell: UICollectionViewCell {
  var key: String = ""
  
  required init?(coder aDecoder: NSCoder) {
    super.init(coder: aDecoder)
  }
  
  override init(frame: CGRect) {
    super.init(frame: frame)
  }
}

class PageChooser : UICollectionViewController {
  var firebaseProvider = UIApplication.sharedApplication().delegate as! FirebaseRefProvider
  let ref : Firebase
  var query : UInt = 0
  var pages = [String]()

  required init?(coder aDecoder: NSCoder) {
    self.ref = firebaseProvider.getFirebaseRef().childByAppendingPath("/pageList")    
    super.init(coder: aDecoder)
  }
  
  override func viewDidLoad() {
    super.viewDidLoad()
    self.collectionView!.registerClass(PageCell.self, forCellWithReuseIdentifier: "CELL")
    let layout = self.collectionViewLayout as! UICollectionViewFlowLayout
    layout.itemSize = CGSize(width: 240, height: 180)
    layout.sectionInset = UIEdgeInsetsMake(0, 10, 0, 10)
  }
  
  override func viewWillAppear(animated: Bool) {
    super.viewWillAppear(animated)
    
    query = ref.observeEventType(.ChildAdded, withBlock: { (snapshot: FDataSnapshot!) -> Void in
      print("received snapshow with value: \(snapshot.value) \(snapshot.key)")
      self.pages.append(snapshot.key)
      self.collectionView!.reloadData()
    });
  }
  
  override func viewWillDisappear(animated: Bool) {
    super.viewWillDisappear(animated)
    ref.removeObserverWithHandle(query)
    query = 0
    self.pages.removeAll()
  }
  
  override func numberOfSectionsInCollectionView(collectionView: UICollectionView) -> Int {
    return 1
  }
  
  override func collectionView(collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
    return pages.count
  }
  
  override func collectionView(collectionView: UICollectionView,
                               cellForItemAtIndexPath indexPath: NSIndexPath) -> UICollectionViewCell {
    let cell = collectionView.dequeueReusableCellWithReuseIdentifier("CELL", forIndexPath: indexPath) as! PageCell
    cell.key = pages[indexPath.item]
    cell.backgroundColor = UIColor(red: 0.2, green: 0.0, blue: 0.2, alpha: 1.0)
    return cell
  }
  
  override func collectionView(collectionView: UICollectionView, didSelectItemAtIndexPath indexPath: NSIndexPath) {
    firebaseProvider.pageId = pages[indexPath.item]
    self.tabBarController!.selectedIndex = 0
  }
}
