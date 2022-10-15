#version 130

#ifdef GL_ES
precision mediump float;
#endif

// Scene data
uniform mat4 model_matrix;
// Fog data
uniform int fog_active;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
// Lighting data
uniform vec3 sun_direction;
uniform vec3 light_diffuse;
uniform vec3 light_ambient;
// Shadow data
uniform sampler2D wtex;
uniform sampler2D shadow_cascade0;
uniform sampler2D shadow_cascade1;
uniform sampler2D shadow_cascade2;
uniform mat4 cascade_mvp[3];
uniform int shadowtex_size;

// From fragment shader
varying vec3 worldpos;
varying vec4 fcolor;
varying vec2 texco;
varying vec3 cpos;
varying vec3 inormal;
varying float flatkey;

float cascadeShadow() {
  vec4 pos = vec4(worldpos.x, worldpos.y, worldpos.z, 1.);
  float off = 1. / shadowtex_size;
  /* smallest layer is first */
  int layer = 2;
  vec4 qp;
  vec2 shadco;
  for (int texidx = 0; texidx < 2; texidx++) {
    mat4 mvp_matrix = cascade_mvp[texidx];
    qp = mvp_matrix * pos;
    shadco = qp.xy * 0.5 + 0.5;
    layer = 0;
    if (shadco.x > off && shadco.x < 1.0 - off && shadco.y > off && shadco.y < 1.0 - off) {
      layer = texidx;
      break;
    }
  }
  float shadow = 0.;
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      vec2 tr = vec2(dx * off, dy * off);
      float dpth;
      if (layer == 0) {
        dpth = 2 * texture(shadow_cascade0, shadco + tr).r - 1;
      } else if (layer == 1) {
        dpth = 2 * texture(shadow_cascade1, shadco + tr).r - 1;
      } else {
        dpth = 2 * texture(shadow_cascade2, shadco + tr).r - 1;
      }

      shadow += (dpth < qp.z - 0.001) ? 0.0 : 1.0;
    }
  }
  return shadow / 9.0;
}

void main(void) {
  vec4 fcolor = vec4(0.3, 0.3, 0.7, 0.6);
  vec3 normal = normalize(inormal);
  vec4 texcolor = fcolor * texture2D(wtex, texco);

  // Determine Day Lighting parameters
  vec3 L = normalize(-mat3(model_matrix) * sun_direction);
  float source_strength = 0.35 + 0.75 * cascadeShadow();
  if (dot(normal, L) < 0) source_strength = 0.35;

  // Specular-free lighting
  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec3 Iambient = texcolor.xyz * light_ambient;
  vec3 Idiffuse = texcolor.xyz * light_diffuse * max(dot(normal, L), 0.0);
  vec3 surfcolor = Iambient + source_strength * Idiffuse;
  surfcolor = clamp(surfcolor, 0.0, 1.0);

  // Blending into fog
  if (fog_active == 1) {
    float dist = clamp((length(cpos) - fog_start) / (fog_end - fog_start), 0., 1.0);
    surfcolor = mix(surfcolor, fog_color, dist * sqrt(dist));
  }
  // Keep original alpha
  gl_FragColor = vec4(surfcolor, texcolor.w);
}
