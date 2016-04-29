//
//  Arc.swift
//  skeqi
//
//  Created by Josh Gargus on 4/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

import Foundation

import Darwin
let π = Float(M_PI)

public struct Point2d {
  public let x, y : Float
  public init(_ x: Float, _ y: Float) {
    self.x = x
    self.y = y
  }
  
  public init(radians: Float, radius: Float, origin: Point2d = Point2d(0.0, 0.0)) {
    x = radius * cos(radians) + origin.x
    y = radius * sin(radians) + origin.y
  }
}

public prefix func -(point: Point2d) -> Point2d {
  return Point2d(-point.x, -point.y)
}

public func +(p1: Point2d, p2: Point2d) -> Point2d {
  return Point2d(p1.x + p2.x, p1.y + p2.y)
}

public func -(p1: Point2d, p2: Point2d) -> Point2d {
  return Point2d(p1.x - p2.x, p1.y - p2.y)
}

public struct Arc {
  public let radius, startRadians, endRadians : Float
  public let center : Point2d
  
  // Compute length of Arc
  public var length: Float { return abs(startRadians - endRadians) * radius }  // TODO: test this
  
  // Compute 2d cartesian position of start of Arc.
  public var startPoint: Point2d { return Point2d(radians: startRadians, radius: radius, origin: center) }
  
  // Compute 2d cartesian position of end of Arc.
  public var endPoint: Point2d { return Point2d(radians: endRadians, radius: radius, origin: center) }
  
  public init(center: Point2d, radius: Float, startRadians: Float, endRadians: Float) {
      self.center = center
      self.radius = radius
      self.startRadians = startRadians
      self.endRadians = endRadians
  }

  public init(startingAt startPoint: Point2d, radius: Float, startRadians: Float, endRadians: Float) {
      self.center = startPoint - Point2d(radians: startRadians, radius: radius)
      self.radius = radius
      self.startRadians = startRadians
      self.endRadians = endRadians
    }
}

public class ArcList {
  var arcs_ = [Arc]()
  // TODO: shallow copy?
  public var arcs : [Arc] { return arcs_ }
  
  public var length: Float { return arcs.reduce(0) { $0 + $1.length } }
  
  func append(arc: Arc) {
    // If there is a previous Arc, its endPoint/Radians must match the startPoint/Radians of the new Arc.
    // TODO: this assert doesn't compile, and I don't understand why.
    // assert(arcs.isEmpty || 0.0000001 > abs(arcs.last.endPoint - arc.startPoint + arcs.last.endRadians - arc.startRadians))
    assert(arc.startRadians != arc.endRadians)
    assert(arc.radius != 0.0)
    arcs_.append(arc)
  }
  
  public init(startPoint inputPoint: Point2d, startRadians inputRadians: Float, radiusList: [Float], radiansChangeList: [Float]) {
    var startPoint = inputPoint;
    var startRadians = inputRadians;
    assert(radiusList.count >= 1 && radiusList.count == radiansChangeList.count);
    
    for i in 0 ..< radiusList.count {
      let endRadians = startRadians + radiansChangeList[i]
      let arc = Arc(startingAt: startPoint, radius: radiusList[i], startRadians: startRadians, endRadians: endRadians)
      append(arc)
      startRadians = endRadians
      startPoint = arc.endPoint
    }
  }
}
