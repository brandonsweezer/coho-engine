#include "ResourceLoader.h"
#include "tiny_obj_loader.h"
#include "tiny_gltf.h"
#include "stb_image.h"
#include <webgpu/webgpu.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
namespace fs = std::filesystem;
using namespace wgpu;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using mat3x3 = glm::mat3x3;


// Auxiliary function for loadTexture
static void writeMipMaps(
	Device device,
	Texture texture,
	Extent3D textureSize,
	uint32_t mipLevelCount,
	const unsigned char* pixelData)
{
	Queue queue = device.getQueue();

	// Arguments telling which part of the texture we upload to
	ImageCopyTexture destination;
	destination.texture = texture;
	destination.origin = { 0, 0, 0 };
	destination.aspect = TextureAspect::All;

	// Arguments telling how the C++ side pixel memory is laid out
	TextureDataLayout source;
	source.offset = 0;

	// Create image data
	Extent3D mipLevelSize = textureSize;
	std::vector<unsigned char> previousLevelPixels;
	Extent3D previousMipLevelSize;
	for (uint32_t level = 0; level < mipLevelCount; ++level) {
		// Pixel data for the current level
		std::vector<unsigned char> pixels(4 * mipLevelSize.width * mipLevelSize.height);
		if (level == 0) {
			// We cannot really avoid this copy since we need this
			// in previousLevelPixels at the next iteration
			memcpy(pixels.data(), pixelData, pixels.size());
		}
		else {
			// Create mip level data
			for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
				for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
					unsigned char* p = &pixels[4 * (j * mipLevelSize.width + i)];
					// Get the corresponding 4 pixels from the previous level
					unsigned char* p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char* p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
					unsigned char* p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
					unsigned char* p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
					// Average
					p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
					p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
					p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
					p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
				}
			}
		}

		// Upload data to the GPU texture
		destination.mipLevel = level;
		source.bytesPerRow = 4 * mipLevelSize.width;
		source.rowsPerImage = mipLevelSize.height;
		queue.writeTexture(destination, pixels.data(), pixels.size(), source, mipLevelSize);

		previousLevelPixels = std::move(pixels);
		previousMipLevelSize = mipLevelSize;
		mipLevelSize.width /= 2;
		mipLevelSize.height /= 2;
	}

	queue.release();
}

// overloaded loadTexture -> this one creates a textureView as well.
Texture ResourceLoader::loadTexture(const std::string& path, const std::string& filename, wgpu::Device& device, wgpu::TextureView& texture_view) {
    Texture texture = loadTexture(path, filename, device);

    std::cout << "texture view " << std::endl;
    TextureViewDescriptor texViewDesc;
    texViewDesc.arrayLayerCount = 1;
    texViewDesc.baseArrayLayer = 0;
    texViewDesc.aspect = TextureAspect::All;
    texViewDesc.baseMipLevel = 0;
    texViewDesc.mipLevelCount = 8;
    texViewDesc.dimension = TextureViewDimension::_2D;
    texViewDesc.format = TextureFormat::RGBA8Unorm;
    texViewDesc.label = filename.c_str();
    texture_view = texture.createView(texViewDesc);

    return texture;
}

// overloaded load texture. this one doesn't create a texture view
Texture ResourceLoader::loadTexture(const std::string& path, const std::string& filename, wgpu::Device& device) {
    int width, height, channels;
    auto data = stbi_load(std::string(path + std::string("/") + filename).c_str(), &width, &height, &channels, 4);
    if (data == nullptr) {
        return nullptr;
    }

    TextureDescriptor textureDesc;
    textureDesc.dimension = TextureDimension::_2D;
    textureDesc.format = TextureFormat::RGBA8Unorm; // png/jpeg format
    textureDesc.label = filename.c_str();
    textureDesc.mipLevelCount = 8;
    textureDesc.sampleCount = 1;
    textureDesc.size.width = width;
    textureDesc.size.height = height;
    textureDesc.size.depthOrArrayLayers = 1;
    textureDesc.usage = TextureUsage::CopyDst | TextureUsage::TextureBinding;
    textureDesc.viewFormatCount = 0;
    textureDesc.viewFormats = nullptr;
    Texture texture = device.createTexture(textureDesc);

    writeMipMaps(device, texture, textureDesc.size, textureDesc.mipLevelCount, data);

    stbi_image_free(data);

    return texture;
}

std::string ResourceLoader::loadShaderCode(std::string path) {
    std::ifstream file(path);
	if (!file.is_open()) {
		return nullptr;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);

    return shaderSource;
}

bool ResourceLoader::loadGLTF(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path + std::string("/") + filename, 0);

    if (!warn.empty()) {
        std::cout << "warning: " << warn.c_str() << std::endl;
    }
    if (!err.empty()) {
        std::cout << "err: " << err.c_str() << std::endl;
    }
    if (!ret) {
        std::cout << "failed to load file: " << path << "/" << filename << std::endl;
        return false;
    }

    vertexData.clear();
    for (tinygltf::Mesh mesh : model.meshes) {
        for (tinygltf::Primitive primitive : mesh.primitives) {
            int accessorIndex = primitive.attributes["POSITION"];
            tinygltf::Accessor accessor = model.accessors[accessorIndex];
            tinygltf::BufferView bufferView = model.bufferViews[accessor.bufferView];
            tinygltf::Buffer buffer = model.buffers[bufferView.buffer];
            
        }
    }
    // TODO this doesn't work, and I'd rather spend my time working on adding rendering features rather than parsing files
    return true;
}

bool ResourceLoader::loadObj(const std::string& path, const std::string& filename, std::vector<VertexData>& vertexData) {
    std::cout << "loading .obj file: " << filename << "from " << path << std::endl;

    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = path.c_str();

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path + std::string("/") + filename, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        return false;
    }
    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    // auto& materials = reader.GetMaterials();


    vertexData.clear();
    
    // could be multiple shapes in obj file
    for (auto shape : shapes) {
        // combine all shapes into offset VertexData array
        size_t offset = vertexData.size();
        vertexData.resize(offset + shape.mesh.indices.size());
        for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
            tinyobj::index_t it = shape.mesh.indices[i];
            vertexData[offset + i].position = {
                attrib.vertices[3 * it.vertex_index + 0],
                attrib.vertices[3 * it.vertex_index + 1],
                attrib.vertices[3 * it.vertex_index + 2]
            };
            vertexData[offset + i].normal = {
                attrib.normals[3 * it.normal_index + 0],
                attrib.normals[3 * it.normal_index + 1],
                attrib.normals[3 * it.normal_index + 2]
            };
            vertexData[offset + i].color = {
                attrib.colors[3 * it.vertex_index + 0],
                attrib.colors[3 * it.vertex_index + 1],
                attrib.colors[3 * it.vertex_index + 2]
            };
            vertexData[offset + i].uv = {
                attrib.texcoords[2 * it.texcoord_index + 0],
                1 - attrib.texcoords[2 * it.texcoord_index + 1],
            };
        }
        
    }

    bool success = vertexDataCalculations(vertexData);
    if (!success) {
        std::cout << "failed to do vertex calculations" << std::endl;
    }

    std::cout << "done loading " << path << std::endl;

    return true;
}

mat3x3 ResourceLoader::calculateTBN(VertexData v0, VertexData v1, VertexData v2, vec3 expectedN) {
    vec3 e1 = v1.position - v0.position;
    vec3 e2 = v2.position - v0.position;

    vec2 uv1 = v1.uv - v0.uv;
    vec2 uv2 = v2.uv - v0.uv;

    vec3 T = glm::normalize(e1 * uv2.y - e2 * uv1.y);
    vec3 B = glm::normalize(e2 * uv1.x - e1 * uv2.x);
    vec3 N = glm::cross(T,B);

    if (dot(N, expectedN) < 0.0) {
        T = -T;
        B = -B;
        N = -N;
    }

    // orthonormalize
    N = expectedN;
    T = glm::normalize(T - dot(T, N) * N);
    B = glm::cross(N, T);

    return mat3x3(T,B,N);

}

bool ResourceLoader::vertexDataCalculations(std::vector<VertexData>& vertexData) {
    size_t triangleCount = vertexData.size() / 3;
    for (int i = 0; i < triangleCount; ++i) {
        // calculate tangent and bitangent
        VertexData& v0 = vertexData[3*i + 0];
        VertexData& v1 = vertexData[3*i + 1];
        VertexData& v2 = vertexData[3*i + 2];
        
        glm::mat3x3 TBN0 = calculateTBN(v0, v1, v2, v0.normal);
        v0.tangent = TBN0[0];
        v0.bitangent = TBN0[1];
        v0.normal = TBN0[2];

        glm::mat3x3 TBN1 = calculateTBN(v0, v1, v2, v1.normal);
        v1.tangent = TBN1[0];
        v1.bitangent = TBN1[1];
        v1.normal = TBN1[2];

        glm::mat3x3 TBN2 = calculateTBN(v0, v1, v2, v2.normal);
        v2.tangent = TBN2[0];
        v2.bitangent = TBN2[1];
        v2.normal = TBN2[2];

    }

    return true;
}