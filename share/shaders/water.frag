#version 130

#ifdef GL_ES
precision mediump float;
#endif

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
uniform float quadratic_attenuation;

uniform sampler2D wtex;
uniform samplerCube shadow_map;

varying vec2 texco;
varying vec3 cpos;
varying vec3 inormal;
varying vec3 worldpos;

const float F = 1000.0;
const float N = 0.1;

void main(void) {
  vec4 fcolor = vec4(0.3, 0.3, 0.7, 0.6);

  // Linear combinations of units need not be units
  vec3 normal = normalize(inormal);
  vec4 texcolor = fcolor * texture2D(wtex, texco);

  // Lighting model, not tuned very much
  vec3 light_delta = worldpos - light_position;
  float lightReach = texture(shadow_map, vec3(normalize(light_delta))).r;
  float light_dist = 2*F*N / (F+N-(2*lightReach-1)*(F-N));
  light_dist = light_dist * length(light_delta) / max(abs(light_delta.x), max(abs(light_delta.y), abs(light_delta.z)));
  float shadowFactor = 0.5+0.5*step(length(light_delta), light_dist+1e-2);

  vec3 light_pos = vec4(model_matrix * vec4(light_position, 1.)).xyz;
  float light_distance = length(light_pos - cpos);
  float strength = 1.0 / (1. + quadratic_attenuation * light_distance * light_distance);
  vec3 L = normalize(light_pos - cpos);
  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec3 Iambient = texcolor.xyz * strength * light_ambient;
  vec3 Idiffuse = texcolor.xyz * strength * light_diffuse * max(dot(normal, L), 0.0);
  Idiffuse = clamp(Idiffuse, 0.0, 1.0);
  vec3 surfcolor = light_ambient + Iambient + Idiffuse;
  surfcolor = shadowFactor * clamp(surfcolor, 0.0, 1.0);

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
