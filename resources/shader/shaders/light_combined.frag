#version 110

//the 5 is arbitrary, lights WILL very quickly add up to computational complexity.
//there might be a hardware limitation for the amount of lights you can have, id recommend staying under 8.
//potentially you could go up to 64 of each light. i suspect itll work, but i cant promise
#define MAX_POINT_LIGHTS 5
#define MAX_SPOT_LIGHTS 5

#define USING_LIGHT_COLORS false

varying vec2 fragTexCoord;
varying vec2 texPosition;
uniform sampler2D palette;
uniform sampler2D texture;

uniform float u_px;
uniform float u_darken;
uniform vec2 u_tex_size;
uniform vec2 u_parity;

uniform int palette_size;

//point light variables
uniform int pointlight_count;
#if USING_LIGHT_COLORS
uniform vec4 pointlight_color[MAX_POINT_LIGHTS];
#endif
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
#if USING_LIGHT_COLORS
uniform vec4 spotlight_color[MAX_SPOT_LIGHTS];
#endif
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


// **** end of uniform variables

vec4 source = texture2D(texture, fragTexCoord);
vec4 pixel = texture2D(texture, fragTexCoord);

float cross2D(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

vec2 aspectNormalizedCoord(vec2 uv) {
    float aspect = u_tex_size.x / u_tex_size.y;
    return vec2((uv.x - 0.5) * aspect + 0.5, uv.y);
}

void global_shift(float amount) {
	float f_size = float(palette_size);
	for (int i = 0; i < palette_size; i++) {
		float fi = float(i);
		vec4 swatch = texture2D(palette, vec2(fi / f_size, 0));
		vec4 next = texture2D(palette, vec2((fi + amount) / f_size, 0));
		if (source.rgb == swatch.rgb) { pixel.rgb = next.rgb; }
	}
}

vec4 shift(float amount) {
    vec4 pixel = source;

	for (int i = 0; i < palette_size; i++) {
		float fi = float(i);
		vec4 swatch = texture2D(palette, vec2(fi / float(palette_size), 0));
		if (source.rgb == swatch.rgb) { 
            //need something here to clamp this texture pull to the border, if its not already there
            fi = clamp((fi + amount) / float(palette_size), 0.0, 1.0);
			pixel = texture2D(palette, vec2(fi, 0)); 
		}
	}
    return pixel;
}

vec4 saturateColor(vec4 inputColor, float saturationAmount) {
	float gray = dot(inputColor.rgb, vec3(0.2126, 0.7152, 0.0722)); // Using standard luminance weights
	vec3 saturatedColor = mix(vec3(gray), inputColor.rgb, saturationAmount);
	return vec4(clamp(saturatedColor, 0.0, 1.0), inputColor.a);
}

float CalculatePointLightShift(vec2 point, int light) {
    float lightDistance = length(point - pointlight_position[light]);
    if(lightDistance == 0.0 || lightDistance > pointlight_radius[light]) {
        return 0.0;
    }
    lightDistance = lightDistance / (pointlight_radius[light] * pointlight_distance_scaling[light] + pointlight_distance_flat[light]);

    float attenuation = 1.0 / 
                    (
                        pointlight_attenuation_constant[light] +
                        pointlight_attenuation_linear[light] * lightDistance +
                        pointlight_attenuation_quadratic[light] * lightDistance * lightDistance
                    );
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
    if(spotDot < spotlight_outerCutoff[light]){
        return 0.0;
    }
    float spotValue = smoothstep(spotlight_outerCutoff[light], spotlight_cutoff[light], spotDot);
    //float spotAttenuation = pow(spotValue, spotlight_exponent[light]); // i need to dick with this to see what it does

    lightDistance = lightDistance / spotlight_radius[light] * spotlight_distance_scaling[light] + spotlight_distance_flat[light];

    float attenuation = 1.0 / 
                    (
                        spotlight_attenuation_constant[light] +
                        spotlight_attenuation_linear[light] * lightDistance +
                        spotlight_attenuation_quadratic[light] * lightDistance * lightDistance
                    );
    return spotlight_luminence[light] * attenuation * spotValue;
}

void main() {

	vec2 pixelPoint = aspectNormalizedCoord(vec2(fragTexCoord.x, 1.0 - fragTexCoord.y));
	gl_FragColor = gl_Color * pixel;

	float highest_amount = 0.0;
	for(int light = 0; light < pointlight_count; light++){
        highest_amount = max(highest_amount, CalculatePointLightShift(pixelPoint, light));
    }
    for(int light = 0; light < spotlight_count; light++){
        highest_amount = max(highest_amount, CalculateSpotLightShift(pixelPoint, light));
    }
    
    float fraction = fract(highest_amount);
    highest_amount = highest_amount - fraction;
	
	vec2 texPoint = floor(pixelPoint * u_tex_size);
	if(highest_amount >= 1.0) {
		float t = mod(texPoint.x, 2.0);
		float u = mod(texPoint.y, 2.0);
		vec2 t_comp = vec2(0.0, 1.0);
		vec2 u_comp = vec2(0.0, 1.0);
	if(fraction < 0.1) {
		if ((t == t_comp.x && u == u_comp.y) || (t == t_comp.y && u == u_comp.x) || (t == t_comp.y && u == u_comp.y)) { 
            highest_amount -= 1.0;
        }
    } else if(fraction < 0.15) {
		if ((t == t_comp.x && u == u_comp.y ) || (t == t_comp.y && u == u_comp.x)) { 
            highest_amount -= 1.0;
        }
    }
    else if(fraction < 0.2) {
		if ((t == t_comp.x && u == u_comp.x)) { 
			highest_amount -= 1.0;
        }
    }
    }

	
	gl_FragColor = gl_Color * shift(highest_amount - u_darken);
	
}
