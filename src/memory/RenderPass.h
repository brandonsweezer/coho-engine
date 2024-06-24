#pragma once
#include <webgpu/webgpu.hpp>
#include "../ecs/Entity.h"

class RenderPass {
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
    );

};