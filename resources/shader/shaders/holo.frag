#version 110

uniform sampler2D texture;
uniform float u_time;

uniform vec4 u_highlight;
uniform vec4 u_shadow;
uniform float u_glowStrength;

uniform vec2 u_textureSize;

float pixelSize = 2.0;

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

void main()
{
    // -------------------------------------------------
    // 2x pixel grid
    // -------------------------------------------------
    vec2 pixelCoord = floor(gl_TexCoord[0].xy * u_textureSize);

    // -------------------------------------------------
    // sprite UV
    // -------------------------------------------------
    vec2 uv = gl_TexCoord[0].xy;
    pixelCoord = floor(uv * u_textureSize / pixelSize);

    // -------------------------------------------------
    // glitch trigger
    // -------------------------------------------------
    float glitchTime = step(0.90, hash(floor(u_time * 3.0)));

    // -------------------------------------------------
    // pixel-perfect glitch
    // -------------------------------------------------
    float slice = floor(pixelCoord.y / 2.0);
    float shiftPixels = floor((hash(slice + floor(u_time * 6.0)) - 0.5) * 6.0);
    float uvPerPixel = pixelSize / u_textureSize.x;

    if (glitchTime > 0.0) { uv.x += shiftPixels * uvPerPixel; }

    // -------------------------------------------------
    // clamp
    // -------------------------------------------------
    uv = clamp(uv, vec2(0.001), vec2(0.999));

    // -------------------------------------------------
    // sample texture
    // -------------------------------------------------
    vec4 color = texture2D(texture, uv);
    if (glitchTime > 0.0) { color.rgb *= 1.15; }
    if (color.a < 0.01) { discard; }

    // -------------------------------------------------
    // luminance
    // -------------------------------------------------
    float lum = dot(color.rgb, vec3(0.299, 0.587, 0.114));

    // -------------------------------------------------
    // hologram coloration
    // -------------------------------------------------
    vec3 holo = mix(u_shadow.rgb, u_highlight.rgb, lum);
    color.rgb = mix(holo, color.rgb, 0.25);
    color.rgb *= 1.05;

    // -------------------------------------------------
    // glow
    // -------------------------------------------------
    float emission = smoothstep(0.65, 1.0, lum);
    color.rgb += u_highlight.rgb * emission * u_glowStrength;

    // -------------------------------------------------
    // moving scanlines
    // -------------------------------------------------
    float speed = 8.0;
    float phase = floor(u_time * speed);
    float scanRow = floor(pixelCoord.y / 2.0);
    float y = scanRow + phase;
    float stepVal = mod(y, 3.0);

    // -------------------------------------------------
    // occasional dropout
    // -------------------------------------------------
    float dropoutBand = floor(scanRow / 3.0);
    float dropout = step(0.97, hash(scanRow + phase));

    if (dropout > 0.0) {
        color.rgb *= 0.9;
        color.rgb += u_shadow.rgb * 0.05;
    }

    // -------------------------------------------------
    // subtle scanline intensity
    // -------------------------------------------------
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