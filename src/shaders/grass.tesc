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
layout(location = 4) out float out_tessLevel[];  // For LOD visualization

void main() {
    // Don't move the origin location of the patch
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Pass blade data through
    out_v0[gl_InvocationID] = in_v0[gl_InvocationID];
    out_v1[gl_InvocationID] = in_v1[gl_InvocationID];
    out_v2[gl_InvocationID] = in_v2[gl_InvocationID];
    out_up[gl_InvocationID] = in_up[gl_InvocationID];

    // === LOD: Dynamic tessellation based on distance ===
    
    // Get camera position in world space
    vec3 camPos = (inverse(camera.view) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    
    // Get blade position
    vec3 v0 = in_v0[gl_InvocationID].xyz;
    
    // Calculate distance from camera to blade
    float distance = length(v0 - camPos);
    
    // Define LOD distances
    float nearDistance = 5.0;   // High detail
    float midDistance = 15.0;   // Medium detail
    float farDistance = 30.0;   // Low detail
    
    // Calculate tessellation level based on distance
    float tessLevel;
    
    if (distance < nearDistance) {
        // Close: High detail (7 segments)
        tessLevel = 7.0;
    } else if (distance < midDistance) {
        // Medium distance: Interpolate between 7 and 3
        float t = (distance - nearDistance) / (midDistance - nearDistance);
        tessLevel = mix(7.0, 3.0, t);
    } else if (distance < farDistance) {
        // Far: Interpolate between 3 and 1
        float t = (distance - midDistance) / (farDistance - midDistance);
        tessLevel = mix(3.0, 1.0, t);
    } else {
        // Very far: Minimum detail (1 segment)
        tessLevel = 1.0;
    }
    
    // Pass tessellation level for visualization
    out_tessLevel[gl_InvocationID] = tessLevel;
    
    // Apply tessellation levels
    // Outer levels: [left, bottom, right, top]
    gl_TessLevelOuter[0] = 1.0;         // Left edge (width - keep at 1)
    gl_TessLevelOuter[1] = tessLevel;   // Bottom edge (height - varies with distance)
    gl_TessLevelOuter[2] = 1.0;         // Right edge (width - keep at 1)
    gl_TessLevelOuter[3] = tessLevel;   // Top edge (height - varies with distance)
    
    // Inner levels: [horizontal, vertical]
    gl_TessLevelInner[0] = 1.0;         // Horizontal (width - keep at 1)
    gl_TessLevelInner[1] = tessLevel;   // Vertical (height - varies with distance)
}