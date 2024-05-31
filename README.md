# What's this?
Coho is a realtime rendering engine written in C++ using the WebGPU graphics API. It is currently under active development, so just about everything is subject to change.

## Currently Supported Features:
### Instanced rendering
Example: one million perfectly reflective cubes
![image](https://github.com/brandonsweezer/coho-engine/assets/23364714/1946e92b-bf2c-48c9-873a-daaa54857931)

### Material system
Example: from left to right - no material, only a base color, a diffuse texture, a normal texture, a diffuse + normal texture
![image](https://github.com/brandonsweezer/coho-engine/assets/23364714/459c3c2f-e744-4337-a204-811b608f25e0)


Planned Features:
1. Terrain system
2. PBR
3. Shader system


## Web distro?
Not at the moment. I use some native only features of wgpu-native, so you will need to run it natively. 

### Coho? Like the fish? 
you got it.

### Acknowledgements
The [WebGPU distribution](https://github.com/eliemichel/WebGPU-distribution) and [SDL2 integration](https://github.com/eliemichel/sdl2webgpu) used in this project were provided by Elie Michel. Go check out [his wonderful guide](https://github.com/eliemichel/LearnWebGPU) to getting started with C++ development on WebGPU.
