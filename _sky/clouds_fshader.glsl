#version 330 core

in vec3 frag_normal_transformed;
in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform uint lighting_mode;
uniform bool activate_specular;
uniform vec3 spot_direction;
uniform bool activate_spot;
uniform float sky_size;

in vec2 uv_frag;
uniform sampler2D tex_clouds; //wi

uniform vec2 clouds_pos;
// uniform vec3 sun_dir;
uniform vec3 sun_col;
uniform float clouds_level;

in vec3 sun_dir_corr;

out vec4 color;

// const vec3 night_colour = vec3(0.0, 0.0, 0.5);
// const vec3 day_colour = vec3(0.5, 0.5, 1.0);
// const vec3 red_sky = vec3(0.8, 0.3, 0.1);


//parameter cloudiness
// 0.0 : no clouds at all
// 1.0 : really cloudy

void main(){

   vec4 color_cloud = vec4(vec3(1.0), 0.0);

   // color_cloud
   float amount_clouds = texture(tex_clouds, uv_frag+clouds_pos/1000.0).r;
   amount_clouds = amount_clouds*0.5+0.5; //make it [0,1]
   amount_clouds += (clouds_level-0.5)*2;

   float colour_clouds = 1.0;

   if(amount_clouds > 1.0){
      colour_clouds = 1.0-0.5*(amount_clouds-1.0);
   }

   color.rgb = vec3(colour_clouds)*clamp(sun_col*1.5, 0.0, 1.0);

   color.a = clamp(amount_clouds, 0.0, 1.0);
}
