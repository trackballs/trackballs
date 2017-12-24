#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 model_matrix;
uniform vec4 specular;
uniform float shininess;
uniform float use_lighting;
uniform float ignore_shadow;

uniform int fog_active;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
uniform vec3 light_position;
uniform vec3 light_diffuse;
uniform vec3 light_specular;
uniform vec3 light_ambient;
uniform vec3 global_ambient;
uniform float day_mode;
uniform float quadratic_attenuation;

uniform sampler2D tex;
uniform samplerCube shadow_map;
uniform sampler2D shadow_cascade0;
uniform sampler2D shadow_cascade1;
uniform sampler2D shadow_cascade2;
uniform mat4 cascade_proj[3];
uniform mat4 cascade_model[3];

varying vec2 texco;
varying vec3 cpos;
varying vec3 inormal;
varying vec4 fcolor;
varying vec3 worldpos;
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
  // Object shader with custom specular color

  // Linear combinations of units need not be units
  vec3 normal;
  if (flatkey >= 0.) {
    normal = normalize(inormal);
  } else {
    normal = normalize(cross(dFdx(cpos), dFdy(cpos)));
  }
  vec4 texcolor = fcolor * texture(tex, texco);
  if (texcolor.w < 0.001) { discard; }

  // Lighting model, not tuned very much
  float shadowFactor;
  if (day_mode > 0) {
    shadowFactor = 0.5 + 0.5 * cascadeShadow();
  } else {
    shadowFactor = 0.5 + 0.5 * cubeShadow();
  }
  if (ignore_shadow > 0) { shadowFactor = 1.; }

  vec3 light_pos = vec4(model_matrix * vec4(light_position, 1.)).xyz;
  float light_distance = length(light_pos - cpos);
  float strength = 1.0 / (1. + quadratic_attenuation * light_distance * light_distance);
  vec3 L = normalize(light_pos - cpos);
  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec3 Iambient = texcolor.xyz * strength * light_ambient;
  vec3 Idiffuse = texcolor.xyz * strength * light_diffuse * max(dot(normal, L), 0.0);
  Idiffuse = clamp(Idiffuse, 0.0, 1.0);
  vec3 Ispecular =
      specular.xyz * strength * light_specular * pow(max(dot(R, E), 0.0), 0.2 * shininess);
  Ispecular = clamp(Ispecular, 0.0, 1.0);

  vec3 surfcolor = light_ambient + Iambient + Idiffuse + Ispecular;
  if (use_lighting > 0.) {
    surfcolor = shadowFactor * clamp(surfcolor, 0.0, 1.0);
  } else {
    surfcolor = texcolor.xyz;
  }

  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    dist = clamp(1.0 - (fog_end - length(cpos)) / (fog_end - fog_start), 0., 1.0);
  }
  // Force override alpha
  gl_FragColor = vec4(mix(surfcolor, fog_color, dist), texcolor.w);
}
