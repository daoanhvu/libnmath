#version 330

uniform int uUseLighting;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
in vec3 vPos;
in vec4 vColor;
in vec3 vNormal;
out vec4 fragColor;
void main() {
  if(uUseLighting == 1) {
    float distance = length(uLightPos - vPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 lightDir = normalize(uLightPos - vPos);
    vec3 diffuse = max(dot(vNormal, lightDir), 0.0) * uLightColor;
    fragColor = vec4(vColor.rgb * diffuse, vColor.a);
  } else {
    fragColor = vColor;
  }
}