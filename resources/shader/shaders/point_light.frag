#version 110

varying vec2 fragTexCoord;
varying vec2 texPosition;
uniform sampler2D palette;
uniform sampler2D texture;
uniform float u_key;
uniform float u_size;
uniform vec2 u_position;
uniform float u_luminosity;
uniform float u_radius;
uniform vec2 u_offset;

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

vec4 saturateColor(vec4 inputColor, float saturationAmount) {
	float gray = dot(inputColor.rgb, vec3(0.2126, 0.7152, 0.0722)); // Using standard luminance weights
	vec3 saturatedColor = mix(vec3(gray), inputColor.rgb, saturationAmount);
	return vec4(clamp(saturatedColor, 0.0, 1.0), inputColor.a);
}

vec2 isPointInRadius(vec2 point, float dist) {
	float inside = 1.0;
	float dither = 0.0;
	float currdist = u_radius * dist * dist;
	float dither_threshold = currdist * 0.4;
	vec2 diff = vec2(u_position - point);
	if (length(diff) > (currdist + dither_threshold)) {
		inside = 0.0;
	} else if (length(diff) > currdist) {
		float t = mod(point.x, 4.0);
		float u = mod(point.y, 4.0);
		if ((t == 0.0 && u == 2.0) || (t == 2.0 && u == 0.0)) { dither = 1.0; }
	}

	return vec2(inside, dither);
}



void main() {
	float u_px = float(2.0);
	vec2 pixelPoint = gl_FragCoord.xy;
	pixelPoint = floor(pixelPoint / u_px) * u_px;
	int lum = int(u_luminosity);
	bool to_discard = true;
	int len = int(u_luminosity);
	for (int i = len; i >= 0; i--) {
		float fi = float(i);
		vec2 result = isPointInRadius(pixelPoint, fi);
		if (result.x == 1.0) {
			float dither = fi + result.y;
			float amount = u_key - dither;
			shift(amount);
			to_discard = false;
		}
	}
	if (to_discard) { discard; }
	gl_FragColor = gl_Color * pixel;
}