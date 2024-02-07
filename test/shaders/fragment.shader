#version 330 core
// https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Multiple_Lights
// Interpolated values from the vertex shaders
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 Color_vertex;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
flat in int oUseNormal;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
// uniform sampler2D myTextureSampler;
uniform mat4 MV;
uniform vec3 lightPos_worldspace;
uniform vec3 lightColor;

struct LightSource {
  	vec3 position_world;
	vec3 direction_camera;
	vec3 lightColor;
  	vec3 diffuseColor;
  	vec3 ambientColor;
	vec3 specularColor;
};
LightSource light;

void main(){
	// Light emission properties, You probably want to put them as uniforms
	float LightPower = 20.0f;
	float specularPower = 10.0f;

	if(oUseNormal == 1) {
		vec3 MaterialDiffuseColor = Color_vertex;

		// light 1
		light.position_world = lightPos_worldspace;
		light.direction_camera = LightDirection_cameraspace;
		light.specularColor = vec3(0.9,0.9,0.9);
	  	light.ambientColor = vec3(0.4,0.4,0.4) * MaterialDiffuseColor;
		light.lightColor = lightColor;

		vec3 totalLighting = vec3(0.0f, 0.0f, 0.0f);
		// Normal of the computed fragment, in camera space
		vec3 n = normalize( Normal_cameraspace );
		// Eye vector (towards the camera)
		vec3 E = normalize(EyeDirection_cameraspace);
		// vec3 totalLighting = MaterialAmbientColor * 5.0f;
		
		float distance = length( light.position_world - Position_worldspace );
		float distance2 = distance * distance;
		vec3 l = normalize( light.direction_camera );
		float cosTheta = clamp(dot(n, l), 0, 1);
		vec3 R = reflect(-l, n);
		float cosAlpha = clamp( dot( E,R ), 0,1 );
		vec3 diffuseReflection = MaterialDiffuseColor * light.lightColor * LightPower * cosTheta / distance2;
		vec3 specularReflection = light.specularColor * light.lightColor * 3 * specularPower * pow(cosAlpha,5) / distance2;
		totalLighting = totalLighting + light.ambientColor * 3.5f + diffuseReflection + specularReflection;
		color = totalLighting;
	} else {
		color = Color_vertex;
	}
}
