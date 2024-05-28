# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-src"
  "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-build"
  "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix"
  "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/tmp"
  "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp"
  "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src"
  "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Brandon/Documents/github/coho-engine/webgpu/build-wgpu/_deps/webgpu-backend-wgpu-subbuild/webgpu-backend-wgpu-populate-prefix/src/webgpu-backend-wgpu-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
