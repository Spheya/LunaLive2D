#version 430 core

in vec3 screenColor;
in vec4 multiplyColor;
in vec2 uv;
in vec4 clipPos;

uniform sampler2D MainTexture;
uniform sampler2D Live2DMaskTexture;
uniform int Live2DMaskTextureInversed;
uniform float Time;

out vec4 fragColor;

float getMask() {
	float mask = texture(Live2DMaskTexture, (clipPos.xy / clipPos.w) * 0.5 + 0.5).a;
	if(bool(Live2DMaskTextureInversed)) mask = 1.0 - mask;
	return mask;
}

vec3 hsvToRgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 rgbToHsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main() {
	fragColor = texture(MainTexture, uv) * multiplyColor;
	fragColor.rgb = vec3(1.0) - (vec3(1.0) - screenColor) * (vec3(1.0) - fragColor.rgb);
	fragColor.a *= getMask();

	fragColor.rgb = rgbToHsv(fragColor.rgb);
	fragColor.r += Time + clipPos.y / clipPos.w;
	fragColor.rgb = hsvToRgb(fragColor.rgb);

	if(fragColor.a == 0.0) discard;
}