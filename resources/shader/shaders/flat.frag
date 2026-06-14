#version 110

varying vec2 fragTexCoord;
varying vec2 texPosition;

uniform vec4 u_color;
uniform sampler2D u_texture;

vec4 source = texture2D(u_texture, fragTexCoord);
vec4 pixel = texture2D(u_texture, fragTexCoord);

void main() {
	float u_px = float(2.0);
	vec2 pixelPoint = gl_FragCoord.xy;
	pixelPoint = floor(pixelPoint / u_px) * u_px;
    pixel.rgb = u_color.rgb;
	gl_FragColor = gl_Color * pixel;
}