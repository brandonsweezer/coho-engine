#pragma once
#include <webgpu/webgpu.hpp>

namespace coho {
class BindGroup {
public:
    BindGroup();

    ~BindGroup() {
        m_bindGroup.release();
        m_bindGroupLayout.release();
    };

private:
    wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
    wgpu::BindGroup m_bindGroup = nullptr;
};
}