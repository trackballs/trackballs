#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 model_matrix;

uniform int fog_active;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;

uniform vec4 refl_color;

uniform sampler2D tex;

varying vec3 cpos;
varying vec3 inormal;
varying vec3 upvector;

void main(void) {
  // Reflection shader. Texture coordinates are a function of normal direction
  vec3 normal = normalize(inormal);
  vec3 to_camera = normalize(cpos);
  vec3 perp = normalize(cross(to_camera, normalize(upvector)));
  vec3 up = normalize(cross(perp, to_camera));

  float dx = dot(normal, perp);
  float dy = dot(normal, up);

  vec4 texcolor = refl_color * texture(tex, vec2((1 + dx) / 2, (1 + dy) / 2));
  vec3 surfcolor = texcolor.xyz;

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
