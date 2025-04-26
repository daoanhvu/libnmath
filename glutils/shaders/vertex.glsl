#version 330 core
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 uNormalMatrix;

out vec3 vPos;
out vec4 vColor;
out vec3 vNormal;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;
    
    // Transform the position to world space for lighting calculations
    vPos = vec3(worldPos);
    
    // Transform the normal using the normal matrix
    vNormal = normalize(uNormalMatrix * aNormal);
    
    vColor = aColor;
} 