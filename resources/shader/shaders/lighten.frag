#version 110

varying vec2 fragTexCoord;
varying vec2 texPosition;
uniform sampler2D palette;
uniform sampler2D texture;
uniform float u_key;

vec4 source = texture2D(texture, fragTexCoord);
vec4 pixel = texture2D(texture, fragTexCoord);

float cross2D(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

void shift(float amount) {
	int i_size = int(u_size);
	for (int i = 0; i < i_size; i++) {
		float fi = float(i);
		vec4 swatch = texture2D(palette, vec2(fi / u_size, 0));
		vec4 next = texture2D(palette, vec2((fi + amount) / u_size, 0));
		if (source.rgb == swatch.rgb) { pixel.rgb = next.rgb; }
	}
}

void main() {
	float u_px = float(2.0);
	vec2 pixelPoint = gl_FragCoord.xy;
	pixelPoint = floor(pixelPoint / u_px) * u_px;
	shift(u_key);
	gl_FragColor = gl_Color * pixel;
}
