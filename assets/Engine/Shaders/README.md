## Creating shaders
Beneath are guidelines for writing shaders and including them into the project.

### File naming conventions
Shader file names consist of three parts, the name, postfix and file extension.
* The name part can be whatever
* The postfix is dependent on the shader stage (the shader stage names are mostly derived from DirectX):
  * _vs for vertex shaders
  * _hs for hull shaders
  * _ds for domain shaders
  * _gs for geometry shaders
  * _fs for fragment shaders
* The file extension is .hlsl for DirectX 11 shaders, and .glsl for Vulkan shaders.

### Binding slots
In DirectX 11 shaders, samplers should be bound to the same indices as the textures they sample.

The reason for this special rule is that unlike DirectX 11, Vulkan allows samplers and textures to be bound as a single shader resource (combined image-sampler descriptors). Since the graphics API abstraction is favoring Vulkan over DirectX 11, it doesn't acknowledge the concept of textures and samplers being separate. As such, no binding slots are specified for samplers by the API abstraction users. Hence, DirectX shaders should emulate combined image-sampler descriptors using the aforementioned rule.

### Document input layout
Input layouts need to be specified for each vertex shader in ShaderHandler.cpp.

### Compiling shaders
DirectX 11 shaders are compiled at run-time. Vulkan shaders need to be compiled from GLSL to SPIR-V (.spv) each time a shader is added or modified. The latter is done by running the script compile-shaders.py in the tools folder. The script looks for each GLSL file in the Shaders folder and compiles them into SPIR-V.
