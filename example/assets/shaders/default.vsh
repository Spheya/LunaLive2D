#version 430 core

/*
 * THESE SHADERS ARE UNUSED!! THEY JUST EXIST AS A TEMPLATE FOR CUSTOM STUFF
 */

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 ScreenColor;
layout(location = 3) in vec4 MultColor;

out vec3 screenColor;
out vec4 multiplyColor;
out vec2 uv;
out vec4 clipPos;

uniform vec4 MainColor;
uniform vec4 MainTexture_ST;

layout(std140, binding = 0) uniform CameraMatrices {
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};
uniform mat4 ModelMatrix;

void main() {
	screenColor = ScreenColor;
	multiplyColor = MainColor * MultColor;
	uv = UV * MainTexture_ST.xy + MainTexture_ST.zw;
	clipPos = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);
	gl_Position = clipPos;
}