#version 330 core
layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexColor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float useNormal;

out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 Color_vertex;
out float oUseNormal;

void main() {

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  P * V * M * vec4(vertexPosition, 1.0);
	// Position of the vertex, in worldspace : M * position
	Position_worldspace = (M * vec4(vertexPosition, 1.0)).xyz;
	oUseNormal = useNormal;
	Normal_cameraspace = vertexNormal;
	Color_vertex = vertexColor;
}
