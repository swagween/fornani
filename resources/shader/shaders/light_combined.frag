#version 110

//the 5 is arbitrary, lights WILL very quickly add up to computational complexity.
//there might be a hardware limitation for the amount of lights you can have, id recommend staying under 8.
//potentially you could go up to 64 of each light. i suspect itll work, but i cant promise
#define MAX_POINT_LIGHTS 5
#define MAX_SPOT_LIGHTS 5

#define USING_LIGHT_COLORS true

varying vec2 fragTexCoord;
varying vec2 texPosition;
uniform sampler2D palette;
uniform sampler2D texture;

uniform float u_px;
uniform float u_darken;
uniform float u_max_light;
uniform vec2 u_tex_size;
uniform vec2 u_parity;

uniform int palette_size;

//point light variables
uniform int pointlight_count;
uniform vec4 pointlight_color[MAX_POINT_LIGHTS];
uniform vec2 pointlight_position[MAX_POINT_LIGHTS];
uniform float pointlight_radius[MAX_POINT_LIGHTS];
uniform float pointlight_luminence[MAX_POINT_LIGHTS];
uniform float pointlight_attenuation_constant[MAX_POINT_LIGHTS];
uniform float pointlight_attenuation_linear[MAX_POINT_LIGHTS];
uniform float pointlight_attenuation_quadratic[MAX_POINT_LIGHTS];
uniform float pointlight_distance_scaling[MAX_POINT_LIGHTS];
uniform float pointlight_distance_flat[MAX_POINT_LIGHTS];

//spot light variables
uniform int spotlight_count;
uniform vec4 spotlight_color[MAX_SPOT_LIGHTS];
uniform vec2 spotlight_position[MAX_SPOT_LIGHTS];
uniform vec2 spotlight_direction[MAX_SPOT_LIGHTS];
uniform float spotlight_radius[MAX_SPOT_LIGHTS];
uniform float spotlight_luminence[MAX_SPOT_LIGHTS];
uniform float spotlight_attenuation_constant[MAX_SPOT_LIGHTS];
uniform float spotlight_attenuation_linear[MAX_SPOT_LIGHTS];
uniform float spotlight_attenuation_quadratic[MAX_SPOT_LIGHTS];
uniform float spotlight_cutoff[MAX_SPOT_LIGHTS];
uniform float spotlight_outerCutoff[MAX_SPOT_LIGHTS];
uniform float spotlight_distance_scaling[MAX_SPOT_LIGHTS];
uniform float spotlight_distance_flat[MAX_SPOT_LIGHTS];

//sparkle
uniform vec2 u_resolution;
uniform bool u_sparkle_enabled;
uniform sampler2D u_white_noise;
uniform sampler2D u_diagonal_mask;
uniform vec2 u_diagonal_mask_size;
uniform float u_diagonal_mask_pixel_scale;
uniform vec2 u_diagonal_mask_offset;
uniform float u_sparkle_intensity;
uniform vec4 u_sparkle_highlight;

#define MAX_SPARKLE_COLORS 3
#define MAX_SPARKLE_QUALIFYING_COLORS 3

uniform int u_sparkle_colors_count;
uniform vec3 u_sparkle_colors[MAX_SPARKLE_COLORS];

uniform int u_sparkle_qualifying_colors_count;
uniform vec3 u_sparkle_qualifying_colors[MAX_SPARKLE_QUALIFYING_COLORS];

// **** end of uniform variables

vec4 source = texture2D(texture, fragTexCoord);
vec4 pixel = texture2D(texture, fragTexCoord);

float cross2D(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}

int imod(int x, int y) {
    return x - (x / y) * y;
}

vec2 aspectNormalizedCoord(vec2 uv) {
    float aspect = u_tex_size.x / u_tex_size.y;
    return vec2((uv.x - 0.5) * aspect + 0.5, uv.y);
}

bool is_sparkle_color(vec3 color) {
    for(int i = 0; i < u_sparkle_qualifying_colors_count; ++i) {
        if(distance(color, u_sparkle_qualifying_colors[i]) < 0.001) {
            return true;
        }
    }
    return false;
}

void global_shift(float amount) {
    float f_size = float(palette_size);
    for(int i = 0; i < palette_size; i++) {
        float fi = float(i);
        vec4 swatch = texture2D(palette, vec2(fi / f_size, 0));
        vec4 next = texture2D(palette, vec2((fi + amount) / f_size, 0));
        if(source.rgb == swatch.rgb) {
            pixel.rgb = next.rgb;
        }
    }
}

vec4 shift(float amount) {
    vec4 px = source;
    float v = 0.5;

    for(int i = 0; i < palette_size; ++i) {
        float u = (float(i) + 0.5) / float(palette_size);
        vec4 swatch = texture2D(palette, vec2(u, 0.5));

        if(distance(source.rgb, swatch.rgb) < 0.001) {
            float shifted = clamp(float(i) + amount, 0.0, float(palette_size - 1));
            float shiftedU = (shifted + 0.5) / float(palette_size);
            px = texture2D(palette, vec2(shiftedU, 0.5));
            break;
        }
    }
    return px;
}

vec4 saturateColor(vec4 inputColor, float saturationAmount) {
    float gray = dot(inputColor.rgb, vec3(0.2126, 0.7152, 0.0722)); // Using standard luminance weights
    vec3 saturatedColor = mix(vec3(gray), inputColor.rgb, saturationAmount);
    return vec4(clamp(saturatedColor, 0.0, 1.0), inputColor.a);
}

float CalculatePointLightShift(vec2 point, int light) {
    // Compute distance in pixel units (do NOT normalize)
    vec2 lightDir = point - pointlight_position[light];
    float lightDistance = length(lightDir);

    // Early out if too far
    float norm_radius = pointlight_radius[light] / u_tex_size.y;
    if(lightDistance == 0.0 || lightDistance > norm_radius) {
        return 0.0;
    }

    // Inverse quadratic falloff with fixed pixel radius
    float attenuation = pointlight_attenuation_linear[light] * (1.0 - clamp((lightDistance * lightDistance) /
        (norm_radius * norm_radius), 0.0, 1.0));

    return pointlight_luminence[light] * attenuation;
}

float CalculateSpotLightShift(vec2 point, int light) {
    vec2 lightDirection = point - spotlight_position[light];
    float lightDistance = length(lightDirection);

    if(lightDistance == 0.0 || lightDistance > spotlight_radius[light]) {
        return 0.0;
    }
    lightDirection = normalize(lightDirection); //normalized
    float spotDot = dot(lightDirection, -normalize(spotlight_direction[light]));
    if(spotDot < spotlight_outerCutoff[light]) {
        return 0.0;
    }
    float spotValue = smoothstep(spotlight_outerCutoff[light], spotlight_cutoff[light], spotDot);
    //float spotAttenuation = pow(spotValue, spotlight_exponent[light]); // i need to dick with this to see what it does

    lightDistance = lightDistance / spotlight_radius[light] * spotlight_distance_scaling[light] + spotlight_distance_flat[light];

    float attenuation = 1.0 /
        (spotlight_attenuation_constant[light] +
        spotlight_attenuation_linear[light] * lightDistance +
        spotlight_attenuation_quadratic[light] * lightDistance * lightDistance);
    return spotlight_luminence[light] * attenuation * spotValue;
}

bool isPixelShadowed(ivec2 texel, float fraction) {
    int x = imod(texel.x, 2);
    int y = imod(texel.y, 2);

    if(fraction < 0.2) {
        return ((x == 0 && y == 1) || (x == 1 && y == 0) || (x == 1 && y == 1));
    } else if(fraction < 0.3) {
        return ((x == 0 && y == 1) || (x == 1 && y == 0));
    } else if(fraction < 0.4) {
        return (x == 0 && y == 1);
    }

    return false;
}

void main() {

    vec2 uv = gl_TexCoord[0].xy;
    ivec2 texelCoord = ivec2(floor(uv * u_tex_size));
    vec2 texelSize = 1.0 / u_tex_size;
    vec2 snappedUV = (floor(uv * u_tex_size) + 0.5) * texelSize;
    vec2 pixelPoint = aspectNormalizedCoord(vec2(snappedUV.x, 1.0 - snappedUV.y));

    float highest_amount = 0.0;
    vec3 light_color_sum = vec3(0.0);
    float total_light_weight = 0.0;

    for(int light = 0; light < pointlight_count; light++) {
        float amount = CalculatePointLightShift(pixelPoint, light);
        float weight = max(amount, 0.0);

        highest_amount = max(highest_amount, amount);
        light_color_sum += pointlight_color[light].rgb * weight;
        total_light_weight += weight;
    }

    for(int light = 0; light < spotlight_count; light++) {
        float amount = CalculateSpotLightShift(pixelPoint, light);
        float weight = max(amount, 0.0);

        highest_amount = max(highest_amount, amount);
        light_color_sum += spotlight_color[light].rgb * weight;
        total_light_weight += weight;
    }

    vec3 light_color = total_light_weight > 0.0001 ? light_color_sum / total_light_weight : vec3(1.0);

    float fraction = fract(highest_amount);
    highest_amount = highest_amount - fraction;

    if(highest_amount >= 1.0 && isPixelShadowed(texelCoord, fraction)) {
        highest_amount -= 1.0;
    }
    if(highest_amount > u_max_light) {
        highest_amount = u_max_light;
    }

    vec4 shifted = shift(highest_amount - u_darken);

    float tint_amount = pow(clamp(highest_amount / u_max_light, 0.0, 1.0), 2.0) * 0.2;

    vec3 tinted = shifted.rgb * light_color;

    float original_luminance = dot(shifted.rgb, vec3(0.299, 0.587, 0.114));
    float tinted_luminance = dot(tinted, vec3(0.299, 0.587, 0.114));

    tinted *= original_luminance / max(tinted_luminance, 0.0001);

    shifted.rgb = mix(shifted.rgb, tinted, tint_amount);

    vec4 result = shifted;

    if(u_sparkle_enabled && u_sparkle_colors_count > 0) {
        vec2 noise_uv = uv * u_tex_size / u_resolution;
        float noise = texture2D(u_white_noise, noise_uv).r;

        vec2 mask_offset = u_diagonal_mask_offset;
        mask_offset = floor(mask_offset * u_diagonal_mask_size * 0.5 + 0.5) / (u_diagonal_mask_size * 0.5);

        vec2 mask_uv = vec2(uv.x, 1.0 - uv.y) - mask_offset;
        float mask = texture2D(u_diagonal_mask, mask_uv).r;

        float sparkle = smoothstep(0.5, 1.0, noise);
        sparkle *= mask * u_sparkle_intensity;

        if(sparkle > 0.0 && is_sparkle_color(shifted.rgb)) {
            float color_position = fract(noise * 8.0) * 3.0;

            vec3 sparkle_color;

            if(color_position < 1.0) {
                sparkle_color = mix(u_sparkle_colors[0], u_sparkle_colors[1], color_position);
            } else if(color_position < 2.0) {
                sparkle_color = mix(u_sparkle_colors[1], u_sparkle_colors[2], color_position - 1.0);
            } else {
                sparkle_color = mix(u_sparkle_colors[2], u_sparkle_colors[0], color_position - 2.0);
            }

            result.rgb = mix(result.rgb, sparkle_color, sparkle);
        }
    }

    gl_FragColor = gl_Color * result;
}
