#version 330 core

in vec3 position;
in vec2 uv;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;

uniform sampler2D tex_wind; //wind speed texture
uniform float min_pos;
uniform float max_pos;
uniform vec3 sun_dir;
uniform vec3 sun_col;

uniform mat4 shadow_matrix; //bias*P*V
uniform sampler2D shadow_buffer_tex;

uniform vec2 wind_dir;
in vec3 surface_normal; //grass blade normal
in mat4 model_mat;

out vec3 frag_position;
// out vec3 frag_normal_transformed;
// out float frag_diffuse_light;

out float shadow_val;
out vec2 frag_uv;
out vec2 global_uv;
out vec3 frag_light_intensity;

float get_shadow_val(float offset_x, float offset_y, vec4 shadow_coord);

void main(){

   mat4 model = model_mat;
   vec3 new_pos = vec3(model*vec4(position, 1.0));

   vec2 relative_tex_pos = vec2(new_pos.x/100+0.5, new_pos.z/100+0.5);
   global_uv = relative_tex_pos;
   relative_tex_pos /= 4;

   float wind_x = texture(tex_wind, relative_tex_pos+wind_dir ).r;
   float wind_z = texture(tex_wind, relative_tex_pos+vec2(0.5, 0.5)+wind_dir ).r;

   if(gl_VertexID == 0){
      wind_x *= 3.0;
      wind_z *= 3.0;
      new_pos.x += wind_x;
      new_pos.y -= (abs(wind_x)+abs(wind_z))*0.5;
      new_pos.z += wind_z;
   }
   if(gl_VertexID == 1 || gl_VertexID == 2 ){
      wind_x *= 1.5;
      wind_z *= 1.5;
      new_pos.x += wind_x;
      new_pos.y -= (abs(wind_x)+abs(wind_z))*0.5;
      new_pos.z += wind_z;
   }
   if(gl_VertexID == 3 || gl_VertexID == 4 ){
      wind_x *= 0.5;
      wind_z *= 0.5;
      new_pos.x += wind_x;
      new_pos.y -= (abs(wind_x)+abs(wind_z))*0.5;
      new_pos.z += wind_z;
   }

   gl_Position = projection*view*vec4(new_pos, 1.0);

   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 normal_transformed = vec3(0.0);
   normal_transformed = normalize(normalMat*surface_normal);

   vec3 model_pos = vec3(model*vec4(position, 1.0));
   frag_position = model_pos;

   frag_light_intensity = sun_col;

   vec4 shadow_coord = shadow_matrix * model*vec4(position, 1.0);
   shadow_val = get_shadow_val(0.0f, 0.0f, shadow_coord);

   frag_uv = uv;
}

float get_shadow_val(float offset_x, float offset_y, vec4 shadow_coord){
   float shadow = 1.0;
   vec2 offset = vec2(offset_x, offset_y);

   float BIAS = 0.0001;

   vec2 shadow_texture_position = (shadow_coord.xy/shadow_coord.w)+offset;

   if(shadow_texture_position.x > 0.0 && shadow_texture_position.x < 1.0 && shadow_texture_position.y > 0.0 && shadow_texture_position.y < 1.0){
      if ( texture( shadow_buffer_tex, shadow_texture_position ).x  <  shadow_coord.z/shadow_coord.w-BIAS){
          shadow = 0.5;
      }
   }
   return shadow;
}
