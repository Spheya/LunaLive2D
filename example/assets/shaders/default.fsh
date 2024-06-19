#version 430 core

/*
 * THESE SHADERS ARE UNUSED!! THEY JUST EXIST AS A TEMPLATE FOR CUSTOM STUFF
 */

in vec3 screenColor;
in vec4 multiplyColor;
in vec2 uv;
in vec4 clipPos;

uniform sampler2D MainTexture;
uniform sampler2D Live2DMaskTexture;
uniform int Live2DMaskTextureInversed;

out vec4 fragColor;

float getMask() {
	float mask = texture(Live2DMaskTexture, (clipPos.xy / clipPos.w) * 0.5 + 0.5).a;
	if(bool(Live2DMaskTextureInversed)) mask = 1.0 - mask;
	return mask;
}

void main() {
	fragColor = texture(MainTexture, uv) * multiplyColor;
	fragColor.rgb = vec3(1.0) - (vec3(1.0) - screenColor) * (vec3(1.0) - fragColor.rgb);
	fragColor.a *= getMask();

	if(fragColor.a == 0.0) discard;
}