#version 330 core
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 Color_vertex;
in float oUseNormal;

uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;
uniform vec3 lightColor;

out vec4 FragColor;

struct LightSource {
  	vec3 position_world;
	vec3 direction_camera;
	vec3 lightColor;
  	vec3 diffuseColor;
  	vec3 ambientColor;
	vec3 specularColor;
};
LightSource light;

void main() {
	// Light emission properties, You probably want to put them as uniforms
	float LightPower = 1.2f;
	// float specularPower = 10.0f;

	if(oUseNormal > 0.0) {
		vec3 objectColor = Color_vertex;
		if (!gl_FrontFacing) {
			objectColor = objectColor * 0.8;
		}

		light.position_world = lightPos;
	 	light.ambientColor = LightPower * lightColor;
		light.lightColor = lightColor;
		vec3 tempNormal = ( V * M * vec4(Normal_cameraspace, 0.0)).xyz;
		// vec3 inverseNormal = mat3(transpose(inverse(M))) * Normal_cameraspace;
		// Normal of the computed fragment, in camera space
		vec3 norm = normalize(tempNormal);
		vec3 lightDir = normalize(light.position_world - Position_worldspace);
		light.diffuseColor = max(dot(norm, lightDir), 0.0) * light.lightColor;
		FragColor = vec4((light.ambientColor + light.diffuseColor) * objectColor, 1.0);
	} else {
		FragColor = vec4(Color_vertex, 1.0);
	}
}
