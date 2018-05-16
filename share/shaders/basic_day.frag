#version 130

#ifdef GL_ES
precision mediump float;
#endif

// Scene data
uniform int render_stage;
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
uniform sampler2DArray arrtex;
uniform sampler2D shadow_cascade0;
uniform sampler2D shadow_cascade1;
uniform sampler2D shadow_cascade2;
uniform mat4 cascade_mvp[3];

// From fragment shader
varying vec3 worldpos;
varying vec4 fcolor;
varying vec3 texco;
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
  // Drop transparent elements in first stage only
  if (render_stage == 0 && fcolor.w < 0.95) { discard; }

  // Identify normal and texture color
  vec3 normal;
  if (flatkey >= 0.) {
    normal = normalize(inormal);
  } else {
    normal = normalize(cross(dFdx(cpos), dFdy(cpos)));
  }
  float clamped = max(0, min(8, floor(16. * texco.z + 0.5)));
  vec4 texcolor = fcolor * texture(arrtex, vec3(texco.xy, clamped));

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
    float dist = clamp(1.0 - (fog_end - length(cpos)) / (fog_end - fog_start), 0., 1.0);
    surfcolor = mix(surfcolor, fog_color, dist);
  }
  // Keep original alpha
  gl_FragColor = vec4(surfcolor, fcolor.w);
}