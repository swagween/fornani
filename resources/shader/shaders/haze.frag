#version 110

uniform float uTime;
uniform float uParallax;
uniform vec2 uResolution;
uniform vec2 uCamera;
uniform vec3 uHighlightColor;
uniform vec3 uShadowColor;

////////////////////////////////////////////////////////////

float hash(vec2 p)
{
    return fract(
        sin(dot(p, vec2(127.1, 311.7))) * 43758.5453
    );
}

float bayer4x4(vec2 p)
{
    int x = int(mod(p.x, 4.0));
    int y = int(mod(p.y, 4.0));
    int i = x + y * 4;

    float m[16];
    
    m[0]=0.0;  m[1]=8.0;  m[2]=2.0;  m[3]=10.0;
    m[4]=12.0; m[5]=4.0;  m[6]=14.0; m[7]=6.0;
    m[8]=3.0;  m[9]=11.0; m[10]=1.0; m[11]=9.0;
    m[12]=15.0;m[13]=7.0; m[14]=13.0;m[15]=5.0;

    return m[i] / 16.0;
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x)
         + (c - a) * u.y * (1.0 - u.x)
         + (d - b) * u.x * u.y;
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;

    for(int i = 0; i < 5; ++i)
    {
        value += noise(p) * amplitude;

        p *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}

////////////////////////////////////////////////////////////

void main()
{
    ////////////////////////////////////////////////////////
    // SCREEN UV
    ////////////////////////////////////////////////////////
	
	vec2 pixelSize = vec2(2.0);

// camera FIRST (world space shift)
float camScale = uResolution.x;
vec2 cam = uCamera * uParallax / camScale;

// base UV in screen space
vec2 uv = gl_FragCoord.xy / uResolution.xy;
uv.x *= uResolution.x / uResolution.y;

// apply camera BEFORE quantization
uv += cam;

// now pixelate in world-space UV
uv = floor(uv * uResolution.xy / pixelSize) * pixelSize / uResolution.xy;
    ////////////////////////////////////////////////////////


vec2 t = vec2(uTime * 0.04, uTime * 0.03); // slower time

vec2 uvw = uv * 1.2; // bigger fog structures (was 2.0)

// first warp layer (softer, larger)
vec2 p1;
p1.x = fbm(uvw + vec2(0.0, 0.0) + t);
p1.y = fbm(uvw + vec2(5.2, 1.3) + t);

// second warp layer (reduced intensity, smoother motion)
vec2 p2;
p2.x = fbm(uvw + p1 * 0.8 + vec2(2.7, 9.2) - t * 0.3);
p2.y = fbm(uvw + p1 * 0.8 + vec2(8.3, 1.8) + t * 0.25);

// final noise (softer scale)
float n = fbm(uvw + p2 * 1.2);

    ////////////////////////////////////////////////////////

n = smoothstep(0.2, 0.9, n);

   float alpha = n * 0.9;

// quantize
float quantize_factor = 6.0;
alpha = floor(alpha * quantize_factor) / quantize_factor;
float nq = floor(alpha * quantize_factor) / quantize_factor;

// enforce minimum visibility
alpha = max(alpha, 0.08);

vec3 finalColor = mix(uShadowColor, uHighlightColor, nq);
gl_FragColor = vec4(finalColor, alpha);

}
