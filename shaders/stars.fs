#version 330

uniform float time;
uniform float screenWidth;
uniform float screenHeight;

out vec4 finalColor;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453123);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(
        mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x),
        mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x),
        u.y
    );
}

float fbm(vec2 x) {
    float v = 0.0;
    float a = 0.5;
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.5));
    
    for (int i = 0; i < 6; i++) {
        v += a * noise(x);
        x = rot * x * 2.0 + vec2(100.0);
        a *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    uv = uv * 2.0 - 1.0;
    uv.x *= screenWidth / screenHeight;
    
    // Slower, more flowing animation
    vec2 animatedUV = uv + vec2(
        sin(time * 0.03) * 0.1, 
        cos(time * 0.04) * 0.1
    );
    
    // Multiple layers of nebula with different scales and movements
    float nebula1 = fbm(animatedUV * 1.2 + time * 0.01);
    float nebula2 = fbm(animatedUV * 1.8 - time * 0.015);
    float nebula3 = fbm(animatedUV * 2.2 + time * 0.02);
    float nebula4 = fbm(animatedUV * 0.8 - time * 0.005);
    
    // Rich nebula colors
    vec3 nebulaColor1 = vec3(0.15, 0.05, 0.2);    // Purple
    vec3 nebulaColor2 = vec3(0.05, 0.15, 0.2);    // Blue
    vec3 nebulaColor3 = vec3(0.2, 0.05, 0.15);    // Magenta
    vec3 nebulaColor4 = vec3(0.1, 0.1, 0.2);      // Deep blue
    
    // Dark background
    vec3 backgroundColor = vec3(0.01, 0.01, 0.03);
    
    // Combine nebulas with enhanced visibility
    vec3 finalColorRGB = backgroundColor + 
        nebulaColor1 * nebula1 * 0.4 + 
        nebulaColor2 * nebula2 * 0.3 + 
        nebulaColor3 * nebula3 * 0.25 + 
        nebulaColor4 * nebula4 * 0.35;
    
    // Subtle color pulsing
    finalColorRGB *= 1.0 + 0.08 * sin(time * 0.03);
    
    finalColor = vec4(finalColorRGB, 1.0);
}