#version 130

#ifdef GL_ES
precision mediump float;
#endif

// Scene data
uniform mat4 model_matrix;
// Object data
uniform vec4 specular;
uniform float sharpness;
uniform int use_lighting;
uniform int ignore_shadow;
// Fog data
uniform int fog_active;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
// Lighting data
uniform vec3 sun_direction;
uniform vec3 light_diffuse;
uniform vec3 light_ambient;
uniform vec3 light_specular;
// Shadow data
uniform sampler2D tex;
uniform sampler2D shadow_cascade0;
uniform sampler2D shadow_cascade1;
uniform sampler2D shadow_cascade2;
uniform mat4 cascade_mvp[3];

// From fragment shader
varying vec3 worldpos;
varying vec4 fcolor;
varying vec2 texco;
varying vec3 cpos;
varying vec3 inormal;
varying float flatkey;

float cascadeShadow() {
  vec4 pos = vec4(worldpos.x, worldpos.y, worldpos.z, 1.);
  for (int texidx = 0; texidx < 3; texidx++) {
    mat4 mvp_matrix = cascade_mvp[texidx];
    vec4 qp = mvp_matrix * pos;
    vec2 shadco = qp.xy * 0.5 + 0.5;
    if (-0.99 <= qp.x && qp.x <= 0.99 && -0.99 <= qp.y && qp.y <= 0.99) {
      float dpth;
      if (texidx < 0.5) {
        dpth = 2 * texture(shadow_cascade0, shadco).r - 1;
      } else if (texidx < 1.5) {
        dpth = 2 * texture(shadow_cascade1, shadco).r - 1;
      } else {
        dpth = 2 * texture(shadow_cascade2, shadco).r - 1;
      }
      if (dpth < qp.z - 0.001) { return 0.0; }
      return 1.0;
    }
  }
  return 1.0;
}

void main(void) {
  // Identify normal and texture color
  vec3 normal;
  if (flatkey >= 0.) {
    normal = normalize(inormal);
  } else {
    normal = normalize(cross(dFdx(cpos), dFdy(cpos)));
  }
  vec4 texcolor = fcolor * texture(tex, texco);
  if (texcolor.w < 0.001) { discard; }

  vec3 surfcolor;
  if (use_lighting == 1) {
    // Determine Day Lighting parameters
    vec3 L = normalize(-mat3(model_matrix) * sun_direction);
    float source_strength = 0.35 + 0.75 * cascadeShadow();
    if (dot(normal, L) < 0) source_strength = 0.35;
    if (ignore_shadow == 1) source_strength = 1.0;

    // Lighting
    vec3 E = normalize(-cpos);
    vec3 H = normalize(L + E);
    vec3 Iambient = texcolor.xyz * light_ambient;
    vec3 Idiffuse = texcolor.xyz * light_diffuse * max(dot(normal, L), 0.0);
    vec3 Ispecular = specular.xyz * light_specular * pow(max(dot(normal, H), 0.0), sharpness);
    surfcolor = Iambient + source_strength * Idiffuse + source_strength * Ispecular;
    surfcolor = clamp(surfcolor, 0.0, 1.0);
  } else {
    surfcolor = texcolor.xyz;
  }
  // Blending into fog
  if (fog_active == 1) {
    float dist = clamp((length(cpos) - fog_start) / (fog_end - fog_start), 0., 1.0);
    surfcolor = mix(surfcolor, fog_color, dist * sqrt(dist));
  }
  // Keep original alpha
  gl_FragColor = vec4(surfcolor, texcolor.w);
}
