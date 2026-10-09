#version 330 core
uniform int uUseLighting;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform int uSelected;
uniform vec4 uHighlightColor;

in vec3 vPos;
in vec4 vColor;
in vec3 vNormal;
out vec4 FragColor;

void main()
{
    vec4 baseColor = uSelected > 0 ? uHighlightColor : vColor;
    if(uUseLighting > 0) {
      // Ambient lighting
      float ambientStrength = 0.3;
      vec3 ambient = ambientStrength * uLightColor;

      // Get the normal - flip it if we're looking at the back face
      vec3 norm = normalize(gl_FrontFacing ? vNormal : -vNormal);

      // Diffuse lighting
      vec3 lightDir = normalize(uLightPos - vPos);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = diff * uLightColor;

      // Specular lighting
      float specularStrength = 0.5;
      vec3 viewDir = normalize(uViewPos - vPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
      vec3 specular = specularStrength * spec * uLightColor;

      // Combine lighting components
      vec3 result = (ambient + diffuse + specular) * baseColor.rgb;
      FragColor = vec4(result, baseColor.a);
    } else {
      FragColor = baseColor;
    }
} 