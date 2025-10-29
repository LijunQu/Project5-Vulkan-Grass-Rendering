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
layout(location = 3) in float in_tessLevel;

layout(location = 0) out vec4 outColor;

void main() {
    // Uncomment to visualize LOD levels with color:
    // vec3 lodColor;
    // if (in_tessLevel > 6.0) lodColor = vec3(0.0, 1.0, 0.0);      // Green = high detail
    // else if (in_tessLevel > 4.0) lodColor = vec3(1.0, 1.0, 0.0); // Yellow = medium
    // else if (in_tessLevel > 2.0) lodColor = vec3(1.0, 0.5, 0.0); // Orange = low
    // else lodColor = vec3(1.0, 0.0, 0.0);                         // Red = very low
    // outColor = vec4(lodColor, 1.0);
    // return;
    
    // Golden/Yellow grass colors (like wheat or dry grass)
    vec3 baseColor = vec3(0.6, 0.5, 0.1);      // Dark golden brown at base
    vec3 tipColor = vec3(0.95, 0.85, 0.3);     // Bright golden yellow at tip
    
    // Interpolate color from base to tip
    vec3 grassColor = mix(baseColor, tipColor, in_uv.y);
    
    // Add some color variation along the width for more natural look
    float widthVariation = sin(in_uv.x * 3.14159) * 0.1;
    grassColor += vec3(widthVariation * 0.1, widthVariation * 0.08, 0.0);
    
    // Simple directional light (sun-like)
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    vec3 normal = normalize(in_normal);
    
    // Lambertian diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);
    
    // Add some ambient lighting
    float ambient = 0.45;
    
    // Combine lighting with slightly warm tone
    float lighting = ambient + diffuse * 0.7;
    
    // Apply lighting to grass color
    vec3 finalColor = grassColor * lighting;
    
    // Add subtle rim lighting for edges (makes grass pop)
    vec3 viewDir = normalize(vec3(inverse(camera.view)[3]) - in_worldPos);
    float rim = pow(1.0 - max(dot(viewDir, normal), 0.0), 3.0);
    finalColor += vec3(0.3, 0.25, 0.1) * rim * 0.3;
    
    // Slightly darken near base for depth
    finalColor *= mix(0.75, 1.0, in_uv.y);
    
    outColor = vec4(finalColor, 1.0);
}