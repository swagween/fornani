#version 110

uniform sampler2D texture;
uniform float u_time;

uniform vec4 u_highlight;
uniform vec4 u_shadow;

float pixelSize = 2.0;

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

void main()
{
    // -------------------------------------------------
    // 2x pixel grid (for scan stability only)
    // -------------------------------------------------
    vec2 pixelCoord =
        floor(gl_FragCoord.xy / pixelSize);

    // -------------------------------------------------
    // SAFE SPRITE UV (IMPORTANT: no atlas reconstruction)
    // -------------------------------------------------
    vec2 uv = gl_TexCoord[0].xy;

    // -------------------------------------------------
    // occasional glitch trigger
    // -------------------------------------------------
    float glitchTime =
        step(0.90, hash(floor(u_time * 3.0)));

    // -------------------------------------------------
    // row-based horizontal slice shift (UV SPACE SAFE)
    // -------------------------------------------------
    float row = floor(uv.y * 200.0);

    float shift =
        (hash(row + floor(u_time * 10.0)) - 0.5) * 0.08;

    if (glitchTime > 0.0)
    {
        uv.x += shift;
    }

    // -------------------------------------------------
    // HARD CLAMP to prevent atlas bleed
    // -------------------------------------------------
    uv = clamp(uv, vec2(0.001), vec2(0.999));

    // -------------------------------------------------
    // sample texture
    // -------------------------------------------------
    vec4 color = texture2D(texture, uv);

    if (glitchTime > 0.0) { color.rgb *= 1.15; }

    if (color.a < 0.01)
        discard;

    // -------------------------------------------------
    // shadow → highlight material mapping
    // -------------------------------------------------
    float lum =
        dot(color.rgb, vec3(0.299, 0.587, 0.114));

    vec3 holo =
    mix(u_shadow.rgb, u_highlight.rgb, lum);

    color.rgb =
    mix(holo, color.rgb, 0.25);
    
    color.rgb *= 1.25;

    // -------------------------------------------------
    // glow (emission)
    // -------------------------------------------------
    float emission =
        smoothstep(0.4, 1.0, lum);

    color.rgb += u_highlight.rgb * emission * 1.2;

    // -------------------------------------------------
    // HARD stepped scanlines (pixel stable)
    // -------------------------------------------------
    float speed = 10.0;

    float y =
        pixelCoord.y + floor(u_time * speed);

    float stepVal =
        mod(y, 3.0);

color.rgb *= 1.05;

color.rgb += u_highlight.rgb * emission * 0.2;

// dropout -----
float dropout =
    step(0.97, hash(pixelCoord.y + floor(u_time * 8.0)));

if (dropout > 0.0)
{
    color.rgb *= 0.8;
}
// -------------

float scanIntensity =
    (stepVal < 1.0) ? 0.92 :
    (stepVal < 2.0) ? 1.00 :
                      1.08;

color.rgb *= scanIntensity;

    // -------------------------------------------------
    // output
    // -------------------------------------------------
    gl_FragColor = color;
}