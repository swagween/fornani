#version 110

uniform vec2 uResolution;
uniform vec3 uHighlightColor;
uniform float uIntensity;
uniform float uTime;
uniform vec2 uCenter;

void main()
{
    // Snap fragment coordinates to a 2x2 pixel grid.// 4x4 pixel blocks
vec2 pixel_coord = floor(gl_FragCoord.xy / 8.0) * 8.0;

    // Convert the screen-space center to OpenGL coordinates.
    vec2 center_pixel = vec2(
        uCenter.x,
        uResolution.y - uCenter.y
    );

    // Calculate distance from the center in pixels.
    float distance_from_center = length(pixel_coord - center_pixel);

    // Normalize by screen height.
    distance_from_center /= uResolution.y;

    // Animate the radius.
    float radius = 1.4 + 0.15 * sin(uTime * 0.5);

    // Divide the radial gradient into discrete bands.
    float band_count = 8.0;
    float band = floor(distance_from_center / radius * band_count) / band_count;
    band = min(max(band, 0.0), 1.0);

    // Apply effect intensity.
    float alpha = uIntensity * band;

    gl_FragColor = vec4(uHighlightColor, alpha);
}