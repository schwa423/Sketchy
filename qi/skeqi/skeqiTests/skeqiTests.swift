//
//  skeqiTests.swift
//  skeqiTests
//
//  Created by Josh Gargus on 4/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

import UIKit
import XCTest

class skeqiTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testArcLength() {
        let π = Float(M_PI)
        var arc : Arc
        arc = Arc(startingAt: Point2d(100.0, 999.0), radius: 100, startRadians: 0, endRadians: 2*π)
        XCTAssertEqualWithAccuracy(2*π*arc.radius, arc.length, 0.0001);
        arc = Arc(startingAt: Point2d(100.0, 999.0), radius: 100, startRadians: 1, endRadians: 1 + 2*π)
        XCTAssertEqualWithAccuracy(2*π*arc.radius, arc.length, 0.0001);
        arc = Arc(startingAt: Point2d(100.0, 999.0), radius: 100, startRadians: 1, endRadians: 1 - 2*π)
        ;
    }
    
/*
    func testArcCenter {
        let π = Float(M_PI)
        for x = -100 ...100 {
            for y = -100 ...100 {
                for radius = 1 ...5 {
                    for radians = 0 ...100 {
                        let startPoint = Point2d(Float(x) / 10.0, Float(y) / 10.0)
                        
                        
                        let arc = Arc(startingAt: startPoint,
                                      radius: Float(radius) / 10.0,
                                      startRadians: π * Float(radians) / 100.0,
                                      endRadians: π/2 + π * Float(radians) / 100.0)
                        
                        let arc2 = Arc(center: arc.center(),
                                       radius: arc.radius,
                                       startRadians: arc.startRadians,
                                       endRadians: arc.endRadians)
                        sdfds
                        XCTAssertEqualWithAccuracy(arc.center.x, arc2.center.xlength, 0.0001)
                        
                        
                        
                    }
                }
            }
        }
    }
*/
    
    func testArcListLength() {
        let π = Float(M_PI)
        var list : ArcList
/*        list = ArcList(startPoint: Point2d(0.0, 0.0), startRadians: 0, radiusList: [100.0, 200.0], radiansChangeList: [2*π, -2*π])

        let arc1 = list.arcs[0]
        let arc2 = list.arcs[1]
        XCTAssertEqualWithAccuracy(2*π*arc1.radius, arc1.length, 0.0001);
        XCTAssertEqualWithAccuracy(2*π*arc2.radius, arc2.length, 0.0001);
        XCTAssertEqualWithAccuracy(2*arc1.length, arc2.length, 0.0001);
        XCTAssertEqualWithAccuracy(list.length, arc1.length + arc2.length, 0.0001);
*/        
        
//        XCTAssertEqualWithAccuracy(2 * π * list[0].radius, arc.length, 0.0001);
    }
    
    func testCircularArcList() {
        let π = Float(M_PI)
        var list = ArcList(startPoint: Point2d(1.0, 0.0),
                           startRadians: 0,
                           radiusList:[1.0, 1.0, 1.0, 1.0],
                           radiansChangeList: [π/2, π/2, π/2, π/2])
        for i in 0...3 {
            let pointDifference = list.arcs[i].endPoint - list.arcs[(i+1) % 4].startPoint
            XCTAssertEqualWithAccuracy(0.0, pointDifference.x, 0.00001);
            XCTAssertEqualWithAccuracy(0.0, pointDifference.y, 0.00001);
        }
    }
    
    func testPerformanceExample() {
        // This is an example of a performance test case.
        self.measureBlock() {
            // Put the code you want to measure the time of here.
        }
    }
    
}
