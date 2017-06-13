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
uniform float quadratic_attenuation;

uniform sampler2DArray arrtex;
uniform samplerCube shadow_map;

varying vec3 worldpos;
varying vec4 fcolor;
varying vec3 texco;
varying vec3 cpos;
varying vec3 inormal;
varying float flatkey;

const float F = 1000.0;
const float N = 0.1;

float VectorToDepthValue(vec3 Vec)
{
    vec3 AbsVec = abs(Vec);
    float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));
    float NormZComp = (F+N) / (F-N) - (2*F*N)/(F-N)/LocalZcomp;
    return (NormZComp + 1.0) * 0.5;
}

void main(void) {

  if (render_stage == 0 && fcolor.w < 0.95) {
    // Drop transparent elements in first stage only.
    // Tolerance is generous since float packing may induce slight errors
    discard;
  }

  // Linear combinations of units need not be units
  vec3 normal;
  if (flatkey >= 1.) {
    normal = normalize(inormal);
  } else {
    normal = normalize(cross(dFdx(cpos), dFdy(cpos)));
  }
  float clamped = max(0, min(8, floor(16. * texco.z + 0.5)));
  vec4 texcolor = fcolor * texture(arrtex, vec3(texco.xy, clamped));

  // Lighting model, not tuned very much

  vec3 light_pos = vec4(model_matrix * vec4(light_position, 1.)).xyz;

  //float shadowFactor = texture(shadow_map, vec4(worldpos - light_position, light_distance / far_light));
  vec3 light_delta = worldpos - light_position;
  float lightReach = texture(shadow_map, vec3(normalize(light_delta))).r;
  float light_dist = 2*F*N / (F+N-(2*lightReach-1)*(F-N));
  light_dist = light_dist * length(light_delta) / max(abs(light_delta.x), max(abs(light_delta.y), abs(light_delta.z)));
  float shadowFactor = 0.5+0.5*step(length(light_delta), light_dist+1e-2);

  float light_distance = length(light_pos - cpos);
  float strength = 1.0 / (1. + quadratic_attenuation * light_distance * light_distance);
  vec3 L = normalize(light_pos - cpos);
  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec3 Iambient = texcolor.xyz * strength * light_ambient;
  vec3 Idiffuse = texcolor.xyz * strength * light_diffuse * max(dot(normal, L), 0.0);
  Idiffuse = clamp(Idiffuse, 0.0, 1.0);
  vec3 surfcolor = light_ambient + Iambient + Idiffuse;
  surfcolor = shadowFactor*clamp(surfcolor, 0.0, 1.0);

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
