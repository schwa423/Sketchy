import UIKit

class FirstViewController: UIViewController {

    let testRunner: TestRunnerObjC = TestRunnerObjC();
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    @IBAction func runTests(sender: AnyObject) {
        testRunner.runTests()
    }
}

