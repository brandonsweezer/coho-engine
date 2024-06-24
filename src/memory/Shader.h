#pragma once
#include <webgpu/webgpu.hpp>
#include "../ResourceLoader.h"

namespace coho {
class Shader {
public:
  Shader(std::string directory, std::string filename, std::shared_ptr<wgpu::Device> device) {
    wgpu::ShaderModuleWGSLDescriptor shaderModuleWGSLDesc;
    std::string shaderCode = ResourceLoader::loadShaderCode(std::string(directory) + "/" + filename);
    shaderModuleWGSLDesc.code = shaderCode.c_str();
    shaderModuleWGSLDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
    shaderModuleWGSLDesc.chain.next = nullptr;
    
    wgpu::ShaderModuleDescriptor shaderModuleDesc;
    shaderModuleDesc.hintCount = 0;
    shaderModuleDesc.hints = nullptr;
    shaderModuleDesc.nextInChain = &shaderModuleWGSLDesc.chain;
    
    m_shaderModule = std::make_shared<wgpu::ShaderModule>(device->createShaderModule(shaderModuleDesc));

  };
  ~Shader() {
    m_shaderModule.reset();
  };
  wgpu::ShaderModule getShaderModule() {
    return *m_shaderModule;
  };

private:  
  std::shared_ptr<wgpu::ShaderModule> m_shaderModule;
};
}