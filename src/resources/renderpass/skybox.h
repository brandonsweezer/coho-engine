#pragma once
#include <webgpu/webgpu.hpp>
#include "../../ecs/Entity.h"
#include "../../ecs/components/MeshComponent.h"

class SkyboxRenderPass {
public:
static void render(
        wgpu::Device device,
        wgpu::TextureView surfaceTextureView,
        wgpu::TextureView depthTextureView,
        wgpu::RenderPipeline renderPipeline,
        wgpu::Buffer vertexBuffer,
        uint32_t vertexBufferSize,
        wgpu::BindGroup bindGroup,
        std::vector<std::shared_ptr<Entity>> entities
    ) {
    wgpu::RenderPassColorAttachment colorAttachment;
    colorAttachment.clearValue = { 0.0, 0.0, 0.0 };
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.view = surfaceTextureView;

    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.depthClearValue = 1.0;
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Load;
    depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false;

    depthStencilAttachment.stencilClearValue = 0;
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Load;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
    depthStencilAttachment.stencilReadOnly = true;
    depthStencilAttachment.view = depthTextureView;

    wgpu::RenderPassDescriptor renderPassDesc;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.occlusionQuerySet = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    wgpu::CommandEncoder commandEncoder = device.createCommandEncoder(wgpu::CommandEncoderDescriptor{});
    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.beginRenderPass(renderPassDesc);
    renderPassEncoder.setPipeline(renderPipeline);
    renderPassEncoder.setVertexBuffer(0, vertexBuffer, 0, vertexBufferSize);
    renderPassEncoder.setBindGroup(0, bindGroup, 0, nullptr);

    // draw
    for (auto e : entities) {
        auto mesh = e->getComponent<MeshComponent>()->mesh;
        renderPassEncoder.draw(mesh->getVertexCount(), 1, mesh->getVertexBufferOffset(), e->getId());

    }

    renderPassEncoder.end();
    wgpu::CommandBuffer commandBuffer = commandEncoder.finish(wgpu::CommandBufferDescriptor{});
    device.getQueue().submit(commandBuffer);
    commandEncoder.release();
    commandBuffer.release();
    renderPassEncoder.release();
}
};