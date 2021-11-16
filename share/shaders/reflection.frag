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

  float dz = 1. - dx * dx - dy * dy;
  if (dz <= 0.) {
    /* not in view; shouldn't happen anyway */
    discard;
  }
  dz = sqrt(dz);
  float dotprod = 2.0 * dz;
  float vx = -2.0 * dx;
  float vy = -2.0 * dy;
  float vz = 1 - 2.0 * dz;
  float h = 0.05 / vz;
  float fx2 = -h * vx;
  float fy2 = +h * vy;
  if (dotprod < 1.0) {
    fy2 = -fy2;
    fx2 = -fx2;
  }
  if (fx2 > 0.5) {
    fx2 = 0.5;
    fy2 = (fy2 / fx2) * 0.5;
  } else if (fx2 < -0.5) {
    fx2 = -0.5;
    fy2 = (fy2 / -fx2) * 0.5;
  }
  if (fy2 > 0.5) {
    fy2 = 0.5;
    fx2 = (fx2 / fy2) * 0.5;
  } else if (fy2 < -0.5) {
    fy2 = -0.5;
    fx2 = (fx2 / -fy2) * 0.5;
  }

  vec4 texcolor = refl_color * texture(tex, vec2((1 + fx2) / 2, (1 + fy2) / 2));
  vec3 surfcolor = texcolor.xyz;

  if (fog_active == 1) {
    float dist = clamp((length(cpos) - fog_start) / (fog_end - fog_start), 0., 1.0);
    surfcolor = mix(surfcolor, fog_color, dist * dist);
  }
  // Force override alpha
  gl_FragColor = vec4(surfcolor, texcolor.w);
}
