#version 110

uniform vec2 uResolution;
uniform vec3 uHighlightColor;
uniform float uIntensity;
uniform float uTime;
uniform vec2 uCenter;
uniform sampler2D uOverlay;
uniform bool uUseOverlay;
uniform float uVignette;
uniform float uRadius;

void main() {
    // Snap fragment coordinates to a 2x2 pixel grid.// 4x4 pixel blocks
    vec2 pixel_coord = floor(gl_FragCoord.xy / 8.0) * 8.0;

    // Convert the screen-space center to OpenGL coordinates.
    vec2 center_pixel = vec2(uCenter.x, uResolution.y - uCenter.y);

    // Calculate distance from the center in pixels.
    float distance_from_center = length(pixel_coord - center_pixel);

    // Normalize by screen height.
    distance_from_center /= uResolution.y;

    float radius = 1.4 + 0.15 * sin(uTime * 0.5);
    if(uUseOverlay) {
        radius = uRadius;
    }

    // Divide the radial gradient into discrete bands.
    float band_count = 8.0;
    float band = floor(distance_from_center / radius * band_count) / band_count;
    band = min(max(band, 0.0), 1.0);
    if(uUseOverlay) {
        band = floor(pow(distance_from_center / radius, 2.0) * band_count) / band_count;
        band = clamp(band, 0.0, 1.0);
    }

    // Apply effect intensity.
    float alpha = uIntensity * band;

    vec3 color = uHighlightColor;
    float output_alpha = alpha;

    vec4 result = vec4(uHighlightColor, alpha);

    if(uUseOverlay) {
        vec2 overlay_uv = vec2(gl_FragCoord.x / uResolution.x, 1.0 - gl_FragCoord.y / uResolution.y);

        vec4 overlay = texture2D(uOverlay, overlay_uv);

        result.rgb = overlay.rgb;
        result.a *= overlay.a;
    }

    gl_FragColor = result;
}