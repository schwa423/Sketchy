//: Playground - noun: a place where people can play

import UIKit
import Darwin

let π = Float(M_PI)
let twopi = 2 * π

var str = "Hello, playground"
var v = foooo()

var foo = twopi * 3

//var foo2 = Foo()
//foo2.mult(100)

let p1 = Point2d(50.0, 50.0)
let p2 = Point2d(21.0, 23.0)
p1 + p2
(p2 - p1).x


var arc = Arc(center: Point2d(100.0, 0.0), radius: 100.0, startRadians: π, endRadians: 0.0)
arc.length

// init(var startPoint: Point2d, var startRadians: Float, var radiusList: [Float], var radiansChangeList: [Float]) {

var arclist = ArcList(startPoint: Point2d(0,0), startRadians: 0, radiusList: [100, 100], radiansChangeList: [π, -π])
arclist.arcs[0].length
arclist.arcs[1].length
arclist.length



//arc.length

//println(arc.length)




