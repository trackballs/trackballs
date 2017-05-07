#ifdef GL_ES
precision mediump float;
#endif

uniform int fog_active;

void main(void) {
  // Lines are simple!
  vec4 linecolor = vec4(0., 0., 0., 1.0);

  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    dist = clamp(1.0 - (gl_Fog.end - gl_FogFragCoord) * gl_Fog.scale, 0., 1.0);
  }
  gl_FragColor = mix(linecolor, gl_Fog.color, dist);
}
