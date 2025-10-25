#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// Input from tessellation evaluation shader
layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec3 in_worldPos;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 outColor;

void main() {
    // Base grass color (green)
    vec3 baseColor = vec3(0.1, 0.6, 0.2);
    vec3 tipColor = vec3(0.4, 0.8, 0.3);
    
    // Interpolate color from base to tip
    vec3 grassColor = mix(baseColor, tipColor, in_uv.y);
    
    // Simple directional light (sun-like)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 normal = normalize(in_normal);
    
    // Lambertian diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Add some ambient lighting so it's not too dark
    float ambient = 0.4;
    
    // Combine lighting
    float lighting = ambient + diffuse * 0.6;
    
    // Apply lighting to grass color
    vec3 finalColor = grassColor * lighting;
    
    // Add slight darkening near the base for depth
    finalColor *= mix(0.7, 1.0, in_uv.y);
    
    outColor = vec4(finalColor, 1.0);
}
