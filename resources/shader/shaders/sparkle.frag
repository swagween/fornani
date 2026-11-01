#version 110

uniform sampler2D u_texture;
uniform sampler2D u_white_noise;
uniform sampler2D u_diagonal_mask;

uniform float u_intensity;
uniform vec4 u_highlight;

uniform vec2 u_resolution;
uniform vec2 u_position;

void main() {
	vec2 uv = gl_TexCoord[0].xy;
	vec2 scene_uv = vec2(uv.x, 1.0 - uv.y);

	vec4 scene = texture2D(u_texture, scene_uv);
	vec2 noise_uv = (scene_uv * u_resolution + u_position) / vec2(480.0, 256.0);

	float noise = texture2D(u_white_noise, noise_uv).r;
	float mask = texture2D(u_diagonal_mask, scene_uv).r;

	float sparkle = smoothstep(0.85, 0.98, noise);
	sparkle *= mask * u_intensity;

	scene.rgb = mix(scene.rgb, u_highlight.rgb, sparkle * u_highlight.a);

	gl_FragColor = scene;
}