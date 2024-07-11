#pragma once
#include <webgpu/webgpu.hpp>
#include "../../memory/RenderPass.h"
#include "../../ecs/Entity.h"
#include "../../ecs/components/MeshComponent.h"

class NoiseVisRenderPass : RenderPass {
public:
static void render(wgpu::Device device,
        wgpu::TextureView surfaceTextureView,
        wgpu::TextureView depthTextureView,
        wgpu::RenderPipeline renderPipeline,
        wgpu::Buffer vertexBuffer,
        uint32_t vertexBufferSize,
        wgpu::Buffer indexBuffer,
        uint32_t indexBufferSize,
        wgpu::BindGroup bindGroup,
        std::shared_ptr<Entity> renderQuad) {
    wgpu::RenderPassColorAttachment renderPassColorAttachment;
    renderPassColorAttachment.clearValue = { 0.0, 0.0, 0.0 };
    renderPassColorAttachment.loadOp = wgpu::LoadOp::Load;
    renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.view = surfaceTextureView;
    
    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.depthClearValue = 1.0;
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false;

    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
    depthStencilAttachment.stencilReadOnly = false;

    depthStencilAttachment.view = depthTextureView;

    wgpu::RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.occlusionQuerySet = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    wgpu::CommandEncoder commandEncoder = device.createCommandEncoder(wgpu::CommandEncoderDescriptor{});
    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.beginRenderPass(renderPassDesc);
    renderPassEncoder.setPipeline(renderPipeline);
    renderPassEncoder.setVertexBuffer(0, vertexBuffer, 0, vertexBufferSize);
    renderPassEncoder.setBindGroup(0, bindGroup, 0, nullptr);
    renderPassEncoder.setIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0, indexBufferSize);

    
    auto mesh = renderQuad->getComponent<MeshComponent>()->mesh;
    renderPassEncoder.drawIndexed(mesh->getIndexCount(), 1, mesh->getIndexBufferOffset(), mesh->getVertexBufferOffset(), renderQuad->getId());

    renderPassEncoder.end();
    wgpu::CommandBuffer commandBuffer = commandEncoder.finish(wgpu::CommandBufferDescriptor{});
    device.getQueue().submit(commandBuffer);
    commandEncoder.release();
    commandBuffer.release();
    renderPassEncoder.release();
}
};