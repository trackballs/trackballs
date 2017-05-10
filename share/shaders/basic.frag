#ifdef GL_ES
precision mediump float;
#endif

uniform int fog_active;
uniform int render_stage;

// TODO 1d array!
uniform sampler2D tex;
varying vec4 fcolor;
varying vec2 texco;
varying vec3 cpos;


void main(void) {
  if (render_stage == 0 && fcolor.w < 0.99) {
    // Drop transparent elements in first stage only
    discard;
  }

  vec3 normal = normalize(cross(dFdx(cpos), dFdy(cpos)));
  vec4 texcolor = fcolor * texture2D(tex, texco);

  // Lighting model, not tuned very much
  vec3 L = normalize(gl_LightSource[0].position.xyz - cpos);
  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec4 Iambient = texcolor * gl_LightSource[0].ambient;
  vec4 Idiffuse = texcolor * gl_LightSource[0].diffuse * max(dot(normal, L), 0.0);
  Idiffuse = clamp(Idiffuse, 0.0, 1.0);
  vec4 Ispecular = texcolor * gl_LightSource[0].specular *
                   pow(max(dot(R, E), 0.0), 0.3 * gl_FrontMaterial.shininess);
  Ispecular = clamp(Ispecular, 0.0, 1.0);

  vec4 surfcolor = gl_LightModel.ambient + Iambient + Idiffuse + Ispecular;
  surfcolor = clamp(surfcolor, 0.0, 1.0);

  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
      float fogfudge = 1.3;
      dist = clamp(1.0 - (gl_Fog.end * fogfudge - length(cpos)) * gl_Fog.scale*fogfudge, 0., 1.0);
  }
  // Force override alpha
  gl_FragColor = vec4(mix(surfcolor, gl_Fog.color, dist).xyz, fcolor.w);
}
