#version 450

// Output: Pass UV coordinates to the fragment shader
layout(location = 0) out vec2 outUV;

// Hardcoded positions and UVs for a triangle that covers the screen.
// This is a common trick to avoid needing a VBO for a simple fullscreen quad.
vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

vec2 uvs[3] = vec2[](
    vec2(0.0, 0.0),
    vec2(2.0, 0.0),
    vec2(0.0, 2.0)
);

void main() {
    // Select the position and UV based on which vertex is being processed
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    outUV = uvs[gl_VertexIndex];
}