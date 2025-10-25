#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(quads, equal_spacing, ccw) in;

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
} camera;

// Input from tessellation control shader
layout(location = 0) in vec4 in_v0[];
layout(location = 1) in vec4 in_v1[];
layout(location = 2) in vec4 in_v2[];
layout(location = 3) in vec4 in_up[];

// Output to fragment shader
layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec3 out_worldPos;
layout(location = 2) out vec2 out_uv;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    float u = gl_TessCoord.x;  // Horizontal position (width)
    float v = gl_TessCoord.y;  // Vertical position (height)

    // Get blade data
    vec3 v0 = in_v0[0].xyz;
    vec3 v1 = in_v1[0].xyz;
    vec3 v2 = in_v2[0].xyz;
    vec3 up = in_up[0].xyz;
    
    float orientation = in_v0[0].w;
    float height = in_v1[0].w;
    float width = in_v2[0].w;
    float stiffness = in_up[0].w;
    
    // Compute tangent (perpendicular to up in the orientation direction)
    vec3 tangent = normalize(cross(up, vec3(sin(orientation), 0.0, cos(orientation))));
    
    // Use De Casteljau's algorithm to evaluate the Bezier curve at parameter v
    vec3 a = mix(v0, v1, v);
    vec3 b = mix(v1, v2, v);
    vec3 c = mix(a, b, v);
    
    // Calculate the tangent at point c for normal computation
    vec3 t0 = normalize(v1 - v0);
    vec3 t1 = normalize(v2 - v1);
    vec3 tangentCurve = normalize(mix(t0, t1, v));
    
    // Width varies along the blade - wider at base, narrower at tip
    float currentWidth = width * (1.0 - v);
    
    // Position vertex along the width of the blade
    // u = 0: left edge, u = 1: right edge, u = 0.5: center
    vec3 c0 = c - currentWidth * tangent;
    vec3 c1 = c + currentWidth * tangent;
    vec3 position = mix(c0, c1, u);
    
    // Compute normal (perpendicular to blade surface)
    vec3 normal = normalize(cross(tangent, tangentCurve));
    
    // Flip normal to face camera if needed
    vec3 viewDir = normalize(vec3(inverse(camera.view)[3]) - position);
    if (dot(normal, viewDir) < 0.0) {
        normal = -normal;
    }
    
    // Output
    out_normal = normal;
    out_worldPos = position;
    out_uv = vec2(u, v);
    
    // Transform to clip space
    gl_Position = camera.proj * camera.view * vec4(position, 1.0);
}
