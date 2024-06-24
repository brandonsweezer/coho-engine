#pragma once
#include <webgpu/webgpu.hpp>
#include <vector>
#include "Buffer.h"
#include "Shader.h"


namespace coho {
class Pipeline {
public:
    Pipeline(){};
    ~Pipeline() {
        m_vertexShader.reset();
        m_fragmentShader.reset();
        m_computeShader.reset();

        m_renderPipeline.release();
    };

    void init();
    
protected:
    wgpu::RenderPipeline getRenderPipeline() {
        return m_renderPipeline;
    };

protected:
    std::shared_ptr<coho::Shader> m_vertexShader;
    std::shared_ptr<coho::Shader> m_fragmentShader;
    std::shared_ptr<coho::Shader> m_computeShader;

    wgpu::RenderPipeline m_renderPipeline;
    
private:

};
}