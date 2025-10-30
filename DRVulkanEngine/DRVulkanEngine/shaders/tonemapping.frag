#version 450

// Input: UV coordinates from the vertex shader
layout(location = 0) in vec2 inUV;

// Output: The final LDR color for the screen
layout(location = 0) out vec4 outColor;

// Input: The HDR scene texture rendered in the first pass
layout(set = 0, binding = 0) uniform sampler2D hdrSceneTexture;

// Optional: Use a push constant to control exposure dynamically
layout(push_constant) uniform PushConstants {
    float exposure;
} pushConstants;

void main() {
    // 1. Sample the HDR texture to get the high-range color value.
    // This color can have components greater than 1.0.
    vec3 hdrColor = texture(hdrSceneTexture, inUV).rgb;

    // 2. Apply exposure control.
    // Higher exposure makes the scene brighter.
    vec3 exposedColor = hdrColor;

    // 3. Apply Tone Mapping.
    // This is the core step that maps HDR colors to the LDR range.
    // The Reinhard operator is a simple and popular choice.
    vec3 mappedColor = exposedColor / (exposedColor + vec3(1.0));

    // 4. Apply Gamma Correction.
    // This adjusts the color to look correct on a standard monitor.
    const float gamma = 2.2;
    mappedColor = pow(mappedColor, vec3(1.0 / gamma));

    // Set the final color
    outColor = vec4(mappedColor, 1.0);
}