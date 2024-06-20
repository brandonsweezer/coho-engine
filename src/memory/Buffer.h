#pragma once
#include <webgpu/webgpu.hpp>
#include <string>

namespace coho {
class Buffer {
public:
    Buffer(wgpu::Device device, wgpu::BufferDescriptor descriptor, wgpu::BufferBindingLayout binding, uint32_t size, std::string name);
    ~Buffer();
    wgpu::Buffer getBuffer() { return m_buffer; };
    wgpu::BufferBindingLayout getBindingLayout() { return m_bindingLayout; };
    uint32_t getSize() { return m_size; };
    std::string getName() { return m_name; };
private:
    wgpu::Buffer m_buffer;
    wgpu::BufferBindingLayout m_bindingLayout;
    uint32_t m_size;
    std::string m_name;

};
}