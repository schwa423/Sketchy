//
//  MetalExtensions.swift
//  skeqi
//
//  Created by Josh Gargus on 5/5/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Metal
import MetalKit

extension MTLTexture {
  func bytes() -> UnsafeMutablePointer<Void> {
    let width = self.width
    let height   = self.height
    let rowBytes = self.width * 4
    let p = malloc(width * height * 4)
    
    self.getBytes(p, bytesPerRow: rowBytes, fromRegion: MTLRegionMake2D(0, 0, width, height), mipmapLevel: 0)
    
    return p
  }
  
  func toImage() -> CGImage? {
    let p = bytes()
    
    let pColorSpace = CGColorSpaceCreateDeviceRGB()
    
    let rawBitmapInfo = CGImageAlphaInfo.NoneSkipFirst.rawValue | CGBitmapInfo.ByteOrder32Little.rawValue
    let bitmapInfo:CGBitmapInfo = CGBitmapInfo(rawValue: rawBitmapInfo)
    
    let textureSize = self.width * self.height * 4
    let rowBytes = self.width * 4
    let unusedCallback: CGDataProviderReleaseDataCallback = { optionalPointer, pointer, valueInt in }
    let provider = CGDataProviderCreateWithData(nil, p, textureSize, unusedCallback)
    let cgImageRef = CGImageCreate(self.width, self.height, 8, 32, rowBytes, pColorSpace, bitmapInfo, provider!, nil, true, CGColorRenderingIntent.RenderingIntentDefault)!
    
    return cgImageRef
  }
}

// Return an RGBA texture filled with random data.
func generateRandomTexture(device: MTLDevice, width: Int, height: Int) -> MTLTexture {
  // Generate random bytes.
  let length = width * height * 4
  let randomBuffer = device.newBufferWithLength(length, options: .CPUCacheModeDefaultCache)
  arc4random_buf(randomBuffer.contents(), length);
  
  // Generate texture from the bytes we just generated.
  let descriptor = MTLTextureDescriptor.texture2DDescriptorWithPixelFormat(
    .RGBA8Uint, width: width, height: height, mipmapped: false)
  return randomBuffer.newTextureWithDescriptor(descriptor, offset: 0, bytesPerRow: width * 4)
}

