#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(vertices = 1) out;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// Input from vertex shader
layout(location = 0) in vec4 in_v0[];
layout(location = 1) in vec4 in_v1[];
layout(location = 2) in vec4 in_v2[];
layout(location = 3) in vec4 in_up[];

// Output to tessellation evaluation shader
layout(location = 0) out vec4 out_v0[];
layout(location = 1) out vec4 out_v1[];
layout(location = 2) out vec4 out_v2[];
layout(location = 3) out vec4 out_up[];

void main() {
    // Don't move the origin location of the patch
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Pass blade data through
    out_v0[gl_InvocationID] = in_v0[gl_InvocationID];
    out_v1[gl_InvocationID] = in_v1[gl_InvocationID];
    out_v2[gl_InvocationID] = in_v2[gl_InvocationID];
    out_up[gl_InvocationID] = in_up[gl_InvocationID];

    // Set tessellation levels
    // Higher tessellation = more detail
    // We create a quad per grass blade
    
    // You can make this dynamic based on distance if you want LOD
    // For now, using fixed tessellation
    
    // Vertical tessellation (along blade height)
    gl_TessLevelOuter[0] = 1.0;  // Left edge
    gl_TessLevelOuter[1] = 5.0;  // Bottom edge (height segments)
    gl_TessLevelOuter[2] = 1.0;  // Right edge  
    gl_TessLevelOuter[3] = 5.0;  // Top edge (height segments)
    
    gl_TessLevelInner[0] = 1.0;  // Horizontal (width)
    gl_TessLevelInner[1] = 5.0;  // Vertical (height)
}
