#version 110

varying vec2 fragTexCoord;
varying vec2 texPosition;

uniform vec4 u_color;
uniform sampler2D u_texture;

void main()
{
    vec4 source = texture2D(u_texture, fragTexCoord);
    vec4 pixel = source;

    pixel.rgb = mix(source.rgb, u_color.rgb, u_color.a);

    gl_FragColor = gl_Color * pixel;
}