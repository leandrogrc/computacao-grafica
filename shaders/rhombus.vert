#version 120

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vPos;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vTexCoord = gl_MultiTexCoord0.xy;
    vNormal = gl_Normal;
    vPos = gl_Vertex.xyz;
}
