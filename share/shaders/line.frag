#ifdef GL_ES
precision mediump float;
#endif

uniform int fog_active;

varying vec3 cpos;

void main(void) {
  // Lines are simple!
  vec4 linecolor = vec4(0., 0., 0., 1.0);

  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    float fogfudge = 1.3;
    dist = clamp(1.0 - (gl_Fog.end * fogfudge - length(cpos)) * gl_Fog.scale*fogfudge, 0., 1.0);
  }
  gl_FragColor = vec4(mix(linecolor, gl_Fog.color, dist).xyz, linecolor.w);
}
