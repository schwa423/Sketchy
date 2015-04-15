//
//  Stroke.swift
//  skeqi
//
//  Created by Josh Gargus on 4/12/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

import Foundation

// vertex is:
//  16 bytes for position (4 floats)
//   8 bytes for normals (2 floats)
//   4 bytes for cumulative length (1 float)
//   4 bytes for color (4 uint8)
// total: 32 bytes
struct StrokeVertex : Printable {
    var pX, pY, pZ, pW, nX, nY, length : Float
    var cR, cG, cB, cA : UInt8
//    var color : UInt32
    
    init() {
        pX = 0.0
        pY = 0.0
        pZ = 0.2
        pW = 1.0
        nX = 0.0
        nY = 0.0
        length = 0.0
//        color = 1 + 256 * (1 + 256 * (1 + 256))

        cB = 0
        cG = 1
        cR = 1
        cA = 1
//        color = 0
    }
    
    var description: String {
        return "pos: \(pX)/\(pY)/\(pZ)/\(pW)  normals: \(nX)/\(nY)  length: \(length)"
    }
}

class Stroke {
    let arcList : ArcList
    let vertexSize : Int = 32
    
    init(_ arcs : ArcList) {
        arcList = arcs
    }
    
    func emit_(var target: UnsafeMutablePointer<UInt8>,
        _ radius: Float, _ center: Point2d, _ radians: Float, _ cumulativeLength: Float, _ normalFlip: Float) {
            
            let nX = cos(radians)
            let nY = sin(radians)
            let flipX : Float = nX * normalFlip
            let flipY : Float = nY * normalFlip
            
            let width : Float = 0.02
            
            // Create and populate a struct, then copy it into the vertex buffer.  This is less hassle
            // than writing each individual value directly to the buffer.
            var vertex = StrokeVertex()
            var vertexPtr = withUnsafeMutablePointer(&vertex) {UnsafeMutablePointer<UInt8>($0)}
            vertex.pX = (radius * nX + center.x) + (flipX * width)
            vertex.pY = (radius * nY + center.y) + (flipY * width)
            vertex.pZ = 0.0
            vertex.pW = 1.0
            vertex.nX = flipX
            vertex.nY = flipY
            vertex.length = cumulativeLength
            target.assignFrom(vertexPtr, count: vertexSize)
            println("Emitting even vertex: \(vertex)")
            
            // Update the pointer into the vertex buffer, and update the vertex-struct.
            // Then, copy the struct into the vertex buffer like we did before.
            target = target.advancedBy(vertexSize)
            vertex.pX = vertex.pX - (flipX * width)
            vertex.pY = vertex.pY - (flipY * width)
            vertex.nX = -flipX
            vertex.nY = -flipY
            target.assignFrom(vertexPtr, count: vertexSize)
            println("Emitting odd vertex: \(vertex)")
    }
    
    func tesselate_(var target: UnsafeMutablePointer<UInt8>, totalVertexCount: Int, lengthOffset : Float) {
        assert(totalVertexCount % 2 == 0);
        var generatedVertexCount = 0;
        let totalLength = arcList.length;
        let spacing : Float = totalLength / (Float(totalVertexCount) / 2.0 - 1.0)
        var overlap : Float = 0.0
        
        for arc in arcList.arcs {
            // Properties inherent to the Arc.
            let length = arc.length
            let arcRadians = arc.endRadians - arc.startRadians
            
            let intervals = Int(floor((length - overlap) / spacing))
            let arcVertexCount = intervals + 1

            let intervalRadians = arcRadians * (spacing / length)
            let radians = arc.startRadians + (overlap / length) * (arc.endRadians - arc.startRadians)
//            let radians = arc.startRadians
            
            // Resets with every Arc to maintian accurracy.
            // TODO: might be faster to initialize only once, but I doubt it.
            var cumulativeLength = Float(generatedVertexCount / 2) * spacing + lengthOffset
            let normalFlip : Float = arcRadians < 0 ? -1.0 : 1.0
            
            // Tesselate this Arc.
            let radius = arc.radius
            let center = arc.center
            for i in 0...intervals {
                var theRadians : Float = radians + (intervalRadians * Float(i))
                emit_(target, radius, center, theRadians, cumulativeLength, normalFlip)
                target = target.advancedBy(2 * vertexSize)
                cumulativeLength += spacing
                generatedVertexCount += 2
            }
            
            // Compute the next overlap... the next vertex won't be precisely
            // at the beginning of the next arc, but rather offset by the amount
            // of a vertex-interval that didn't fit in the last arc-tesselation.
            overlap = (overlap + Float(intervals + 1) * spacing) - length
        }
        
        // One last vertex.
        // TODO: explain why this is a sane choice (is it?)
        if (overlap < spacing / 5) {
            let arc = arcList.arcs.last!
            emit_(target,
                arc.radius, arc.center, arc.endRadians,
                totalLength + lengthOffset,
                ((arc.endRadians - arc.startRadians) < 0) ? -1.0 : 0.0);
            generatedVertexCount += 2
        }
        
        assert(generatedVertexCount == totalVertexCount);
    }
}
