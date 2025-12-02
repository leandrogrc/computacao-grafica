#version 120

uniform sampler2D uTexture;
uniform float uTime;

varying vec2 vTexCoord;
varying vec3 vPos;

void main()
{
    // Simple pulsating effect based on time and position
    float pulse = 0.5 + 0.5 * sin(uTime * 2.0 + vPos.y * 5.0);
    
    vec4 texColor = texture2D(uTexture, vTexCoord);
    
    // Add a reddish tint that pulses
    vec3 glow = vec3(0.5, 0.0, 0.0) * pulse;
    
    gl_FragColor = texColor + vec4(glow, 0.0);
}
