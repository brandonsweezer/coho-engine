#include "Buffer.h"

namespace coho {
Buffer::Buffer(std::shared_ptr<wgpu::Device> device, wgpu::BufferDescriptor descriptor, wgpu::BufferBindingLayout binding, uint32_t size, std::string name) {
    m_buffer = device->createBuffer(descriptor);
    m_bindingLayout = binding;
    m_size = size;
    m_name = name;
}
Buffer::~Buffer() {
    m_buffer.destroy();
    m_buffer.release();
}

wgpu::Buffer Buffer::getBuffer() {
    return m_buffer;
}

}
