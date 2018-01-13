//
//  SpinnerLayer.swift
//  iTempest
//
//  Created by Randy Rasmussen on 1/7/18.
//  Copyright © 2018 Randy Rasmussen. All rights reserved.
//

import Foundation
import UIKit

class SpinnerLayer : CAMetalLayer
{
   private var isInitialized : Bool = false;
   private var mtlDevice : MTLDevice! = nil;
   private var pipelineState: MTLRenderPipelineState! = nil;
   private var commandQueue: MTLCommandQueue! = nil;
   private var timer: CADisplayLink! = nil;
   private var rotation : Float = 0;
   private var vertexBuffer: MTLBuffer! = nil;
   private var renderParametersBuffer: MTLBuffer! = nil;

    // stuff specific to the example
    /*private let vertexData:[Float] = [
        0.0, 1.0, 0.0,
        -1.0, -1.0, 0.0,
        1.0, -1.0, 0.0];*/

    func initialize() {
        // never mind if we're already initialized
        if (isInitialized) {
            return;
        }
        
        // create our display-link timer
        timer = CADisplayLink(target: self, selector: #selector(updateSpinner));
        timer.add(to: RunLoop.main, forMode: RunLoopMode.defaultRunLoopMode);
        
        // create our Metal device
        mtlDevice = MTLCreateSystemDefaultDevice();
        
        // create our command queue
        commandQueue = mtlDevice.makeCommandQueue();
        
        // we are the CAMetalLayer, so attach ourself to the device
        self.device = mtlDevice;
        self.pixelFormat = .bgra8Unorm // 3
        self.framebufferOnly = true    // 4
        self.frame = CGRect(x:0, y:0, width:100, height:100);
        
        // stuff specific to the example
        var vertexData : [SpinnerVertex] = [
            SpinnerVertex(position:0, corner:0),
            SpinnerVertex(position:0, corner:1),
            SpinnerVertex(position:0, corner:2)
        ];
        let dataSize = vertexData.count * MemoryLayout.size(ofValue: vertexData[0]);
        vertexBuffer = device!.makeBuffer(bytes: vertexData, length: dataSize, options: []);
        
        // example pipeline
        // 1
        let defaultLibrary = device!.newDefaultLibrary()!;
        let fragmentProgram = defaultLibrary.makeFunction(name: "basic_fragment");
        let vertexProgram = defaultLibrary.makeFunction(name: "basic_vertex");
        
        // 2
        let pipelineStateDescriptor = MTLRenderPipelineDescriptor();
        pipelineStateDescriptor.vertexFunction = vertexProgram;
        pipelineStateDescriptor.fragmentFunction = fragmentProgram;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = .bgra8Unorm;
        
        // 3
        pipelineState = try! device!.makeRenderPipelineState(descriptor: pipelineStateDescriptor);
        
        // mark ourself initialized
        isInitialized = true;
    }
    
   func render() {
      guard let drawable = self.nextDrawable() else { return }
      let renderPassDescriptor = MTLRenderPassDescriptor()
      renderPassDescriptor.colorAttachments[0].texture = drawable.texture
      renderPassDescriptor.colorAttachments[0].loadAction = .clear
      renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColor(red: 0.0, green: 104.0/255.0, blue: 5.0/255.0, alpha: 1.0)
 
      let commandBuffer = commandQueue.makeCommandBuffer();

      // set our render parameters
      rotation += 6;
      if (rotation >= 360)
      {
         rotation -= 360;
      }
      let renderParameters : SpinnerRenderParameters =
         SpinnerRenderParameters(rotation: self.rotation);
      let dataSize = MemoryLayout.size(ofValue: renderParameters);
      renderParametersBuffer = device!.makeBuffer(bytes: [renderParameters], length: dataSize, options: []);

      let renderEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)
      renderEncoder.setRenderPipelineState(pipelineState)
      renderEncoder.setVertexBuffer(vertexBuffer, offset: 0, at: Int(SPINNER_VERTICES_BUFFER));
      renderEncoder.setVertexBuffer(renderParametersBuffer, offset: 0, at: Int(SPINNER_RENDER_PARAMETERS_BUFFER));
      renderEncoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3, instanceCount: 1)
      renderEncoder.endEncoding()
        
      commandBuffer.present(drawable)
      commandBuffer.commit()
        
   }
    
    func updateSpinner() {
        autoreleasepool {
            self.render()
        }
    }
}

