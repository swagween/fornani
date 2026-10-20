#version 110

varying vec2 fragTexCoord;
varying vec2 texPosition;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform int u_blend_mode;

void main()
{
    vec4 source = texture2D(u_texture, fragTexCoord);
    vec4 pixel = source;

    if (u_blend_mode == 1)
    {
        // Additive blending
        pixel.rgb = source.rgb + u_color.rgb * u_color.a;
    }
    else if (u_blend_mode == 2)
    {
        // Screen blending
        vec3 blend = u_color.rgb * u_color.a;
        pixel.rgb = 1.0 - (1.0 - source.rgb) * (1.0 - blend);
    }
    else
    {
        // Default: original opacity-based tint
        pixel.rgb = mix(source.rgb, u_color.rgb, u_color.a);
    }

    gl_FragColor = gl_Color * pixel;
}