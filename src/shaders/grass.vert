#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform ModelBufferObject {
    mat4 model;
};

// Input: Blade data
layout(location = 0) in vec4 v0;
layout(location = 1) in vec4 v1;
layout(location = 2) in vec4 v2;
layout(location = 3) in vec4 up;

// Output to tessellation control shader
layout(location = 0) out vec4 out_v0;
layout(location = 1) out vec4 out_v1;
layout(location = 2) out vec4 out_v2;
layout(location = 3) out vec4 out_up;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    // Pass through blade data to tessellation shaders
    out_v0 = v0;
    out_v1 = v1;
    out_v2 = v2;
    out_up = up;
    
    // Set position to v0 (base of grass blade)
    gl_Position = vec4(v0.xyz, 1.0);
}
