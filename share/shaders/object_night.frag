#version 130

#ifdef GL_ES
precision mediump float;
#endif

// Scene uniforms
uniform mat4 model_matrix;
// Object data
uniform vec4 specular;
uniform float shininess;
uniform int use_lighting;
uniform int ignore_shadow;
// Fog data
uniform int fog_active;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;
// Lighting data
uniform vec3 light_position;
uniform vec3 light_diffuse;
uniform vec3 light_ambient;
uniform vec3 light_specular;
// Shadow data
uniform sampler2D tex;
uniform samplerCube shadow_map;

// From fragment shader
varying vec3 worldpos;
varying vec4 fcolor;
varying vec2 texco;
varying vec3 cpos;
varying vec3 inormal;
varying float flatkey;

float cubeShadow() {
  const float F = 1000.0;
  const float N = 0.1;
  vec3 light_pos = vec4(model_matrix * vec4(light_position, 1.)).xyz;
  vec3 light_delta = worldpos - light_position;
  float lightReach = texture(shadow_map, vec3(normalize(light_delta))).r;
  float light_dist = 2 * F * N / (F + N - (2 * lightReach - 1) * (F - N));
  light_dist = light_dist * length(light_delta) /
               max(abs(light_delta.x), max(abs(light_delta.y), abs(light_delta.z)));
  return step(length(light_delta), light_dist + 1e-2);
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
    // Determine Night Lighting parameters
    vec3 light_pos = vec4(model_matrix * vec4(light_position, 1.)).xyz;
    float light_distance = length(light_pos - cpos);
    vec3 L = normalize(light_pos - cpos);
    float source_strength = 0.5 + 0.5 * cubeShadow();
    if (dot(normal, L) < 0) source_strength = 0.5;
    if (ignore_shadow == 1) source_strength = 1.0;
    source_strength *= 1.0 / (1. + 0.25 * light_distance * light_distance);

    // Lighting
    vec3 E = normalize(-cpos);
    vec3 R = normalize(-reflect(L, normal));
    vec3 Iambient = texcolor.xyz * light_ambient;
    vec3 Idiffuse = texcolor.xyz * light_diffuse * max(dot(normal, L), 0.0);
    vec3 Ispecular = specular.xyz * light_specular * pow(max(dot(R, E), 0.0), 0.2 * shininess);
    surfcolor = Iambient + source_strength * Idiffuse + source_strength * Ispecular;
    surfcolor = clamp(surfcolor, 0.0, 1.0);
  } else {
    surfcolor = texcolor.xyz;
  }

  // Blending into fog
  if (fog_active == 1) {
    float dist = clamp(1.0 - (fog_end - length(cpos)) / (fog_end - fog_start), 0., 1.0);
    surfcolor = mix(surfcolor, fog_color, dist);
  }
  // Keep original alpha
  gl_FragColor = vec4(surfcolor, texcolor.w);
}