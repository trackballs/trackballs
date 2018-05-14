#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform int render_stage;

uniform mat4 model_matrix;
uniform int fog_active;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
uniform vec3 light_position;
uniform vec3 light_diffuse;
uniform vec3 light_specular;
uniform vec3 light_ambient;
uniform vec3 global_ambient;
uniform vec3 sun_direction;
uniform int day_mode;
uniform float quadratic_attenuation;

uniform sampler2DArray arrtex;
uniform samplerCube shadow_map;
uniform sampler2D shadow_cascade0;
uniform sampler2D shadow_cascade1;
uniform sampler2D shadow_cascade2;
uniform mat4 cascade_proj[3];
uniform mat4 cascade_model[3];

varying vec3 worldpos;
varying vec4 fcolor;
varying vec3 texco;
varying vec3 cpos;
varying vec3 inormal;
varying float flatkey;

float cubeShadow() {
  const float F = 1000.0;
  const float N = 0.1;
  vec3 light_pos = vec4(model_matrix * vec4(light_position, 1.)).xyz;

  // float shadowFactor = texture(shadow_map, vec4(worldpos - light_position, light_distance /
  // far_light));
  vec3 light_delta = worldpos - light_position;
  float lightReach = texture(shadow_map, vec3(normalize(light_delta))).r;
  float light_dist = 2 * F * N / (F + N - (2 * lightReach - 1) * (F - N));
  light_dist = light_dist * length(light_delta) /
               max(abs(light_delta.x), max(abs(light_delta.y), abs(light_delta.z)));
  return step(length(light_delta), light_dist + 1e-2);
}

float cascadeShadow() {
  vec4 pos = vec4(worldpos.x, worldpos.y, worldpos.z, 1.);
  for (int texidx = 0; texidx < 3; texidx++) {
    mat4 mvp_matrix = cascade_proj[texidx] * cascade_model[texidx];
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
  if (render_stage == 0 && fcolor.w < 0.95) {
    // Drop transparent elements in first stage only.
    // Tolerance is generous since float packing may induce slight errors
    discard;
  }

  // Linear combinations of units need not be units
  vec3 normal;
  if (flatkey >= 0.) {
    normal = normalize(inormal);
  } else {
    normal = normalize(cross(dFdx(cpos), dFdy(cpos)));
  }
  float clamped = max(0, min(8, floor(16. * texco.z + 0.5)));
  vec4 texcolor = fcolor * texture(arrtex, vec3(texco.xy, clamped));

  // Lighting model, not tuned very much
  vec3 L;
  float source_strength;
  if (day_mode < 0) {
    vec3 light_pos = vec4(model_matrix * vec4(light_position, 1.)).xyz;
    float light_distance = length(light_pos - cpos);
    L = normalize(light_pos - cpos);
    source_strength = 0.5 + 0.5 * cubeShadow();
    if (dot(normal, L) < 0) source_strength = 0.5;
    source_strength *= 1.0 / (1. + quadratic_attenuation * light_distance * light_distance);
  } else {
    L = normalize(-mat3(model_matrix) * sun_direction);
    source_strength = 0.5 + 0.5 * cascadeShadow();
    if (dot(normal, L) < 0) source_strength = 0.5;
  }

  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec3 Iambient = texcolor.xyz * light_ambient;
  vec3 Idiffuse = texcolor.xyz * light_diffuse * max(dot(normal, L), 0.0);
  vec3 surfcolor = light_ambient + Iambient + source_strength * Idiffuse;
  surfcolor = clamp(surfcolor, 0.0, 1.0);

  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    dist = clamp(1.0 - (fog_end - length(cpos)) / (fog_end - fog_start), 0., 1.0);
  }
  // Force override alpha
  gl_FragColor = vec4(mix(surfcolor, fog_color, dist), fcolor.w);
}
