//
//  DetailViewController.swift
//  om_testrunner
//
//  Created by Josh Gargus on 10/11/14.
//  Copyright (c) 2014 schwaftwarez. All rights reserved.
//

import UIKit

class DetailViewController: UIViewController {

    @IBOutlet weak var detailDescriptionLabel: UILabel!
    
    var testRunner: TestRunner? = nil
    
    var detailItem: AnyObject? {
        didSet {
            // Update the view.
            self.configureView()
            
            // Tell the test-runner to run tests.
            testRunner?.runTests()
        }
    }
    
    required init(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder);
        println("****** init()");
    }
    
    func configureView() {
        println("****** configureView()");
        // Update the user interface for the detail item.
        if let detail: AnyObject = self.detailItem {
            if let label = self.detailDescriptionLabel {
                label.text = detail.description + "   WOW, THIS IS FUN!!!!"
            }
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
        self.configureView()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

