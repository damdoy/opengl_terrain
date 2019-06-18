#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;

uniform sampler2D tex;

in float red;
in float green;
in float blue;
out vec4 color;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 shadow_coord;
in vec2 tex_coord;

uniform uint window_width;
uniform uint window_height;
uniform uint shadow_mapping_effect;

uniform vec3 shape_color;

uniform uint shadow_buffer_tex_size;

uniform vec3 sun_dir;
uniform vec3 sun_col;

float get_shadow_val(float offset_x, float offset_y);

mat2 noise_ao;

void main(){

   float shadow = 1;

   //todo shadow map
   // shadow = get_shadow_val(0.0, 0.0);

   vec3 light_dir = normalize(light_position-frag_position);
   float dist_light = distance(light_position, frag_position);
   float diffuse_light = dot(frag_normal, light_dir);
   float light_intensity = diffuse_light*1/(dist_light/8);

   float diffuse_sun = dot(frag_normal, -sun_dir);
   diffuse_sun = clamp(diffuse_sun, 0.0, 1.0);
   vec3 sun_intensity = sun_col*diffuse_sun;

   vec3 final_intensity = vec3(0.0);

   final_intensity.r = sun_intensity.r;
   final_intensity.g = sun_intensity.g;
   final_intensity.b = sun_intensity.b;

   // vec3 texture_col = texture(tex, tex_coord).rgb;
   vec3 texture_col = vec3(0.7, 0.4, 0.2);

   float time_of_day = dot(-sun_dir, vec3(0, 1, 0));

   vec3 color_trunk = texture_col*final_intensity;
   color_trunk = max(color_trunk, texture_col*(time_of_day/4));

   color = vec4( color_trunk*shadow , 1.0);
}

// float get_shadow_val(float offset_x, float offset_y){
//    float shadow = 1.0;
//    vec2 offset = vec2(offset_x, offset_y);
//
//    float BIAS = 0.0001;
//
//    vec2 shadow_texture_position = (shadow_coord.xy/shadow_coord.w)+offset;
//
//    if(shadow_texture_position.x > 0.0 && shadow_texture_position.x < 1.0 && shadow_texture_position.y > 0.0 && shadow_texture_position.y < 1.0){
//       if ( texture( shadow_buffer_tex, shadow_texture_position ).x  <  shadow_coord.z/shadow_coord.w-BIAS){
//           shadow = 0.3;
//       }
//    }
//    return shadow;
// }
