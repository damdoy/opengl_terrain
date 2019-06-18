#version 330 core

in vec3 frag_normal_transformed;
in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform bool activate_colour;
uniform bool activate_heightmap;
uniform bool activate_wireframe;

uniform sampler2D shadow_buffer_tex;
in vec4 shadow_coord;
uniform int shadow_buffer_tex_width;
uniform int shadow_buffer_tex_height;

uniform vec3 sun_dir;
uniform vec3 sun_col;

out vec4 color;

uniform sampler2D tex_grass;
uniform sampler2D tex_sand;
uniform sampler2D tex_snow;
uniform sampler2D tex_rock;

in float frag_nontransfheight;

//basic colours for terrain materials (debug)
const vec3 COLOR[4] = vec3[](
    vec3(0.87, 0.86, 0.5), //sand
    vec3(0.09, 0.46, 0.04), //grass
    vec3(1.0, 1.0, 1.0),   //snow
    vec3(0.5, 0.5, 0.5)); //rock

float get_diffuse_strength(vec3 light_dir, vec3 normal){
   return clamp(dot(normal, light_dir), 0.0, 1.0);
}

float get_shadow_val(float offset_x, float offset_y);

void main(){
   vec3 light_dir = normalize(light_position-frag_position);
   float diffuse_light = 0.0;
   float diffuse_sun = 0.0;
   float spec_light = 0.0;
   float spec_sun = 0.0;
   float light_dist = distance(light_position, frag_position);
   float shadow = 1;

   float distance_to_cam = distance(frag_position, camera_position);

   float ratio_texture = 20.0;

   // vec3 grass_tex = texture(tex_grass, frag_position.xz/ratio_texture).rgb;
   // vec3 sand_tex = texture(tex_sand, frag_position.xz/ratio_texture).rgb;
   // vec3 snow_tex = texture(tex_snow, frag_position.xz/ratio_texture).rgb;
   // vec3 rock_tex = texture(tex_rock, frag_position.xz/ratio_texture).rgb;
   vec3 grass_tex = COLOR[1];
   vec3 sand_tex = COLOR[0];
   vec3 snow_tex = COLOR[2];
   vec3 rock_tex = COLOR[3];

   diffuse_sun = get_diffuse_strength(-sun_dir, frag_normal_transformed);
   float sun_light = 0.8*diffuse_sun;//+spec_sun;

   float height = frag_nontransfheight/2.0+0.5;
   vec3 pixel_colour = vec3(0.0);

   float snow_line = 0.8+sin(sin(frag_position.z*0.05)+frag_position.x*0.15)*0.05;
   float grass_upper = snow_line-0.1;
   float grass_lower = 0.32;
   float sand_line = 0.27;

   if(height > snow_line){
      pixel_colour = snow_tex;
   }
   else if(height > grass_upper && height < snow_line){
      pixel_colour = mix(grass_tex, snow_tex, (height-grass_upper)/(snow_line-grass_upper) );
   }
   else if(height > grass_lower && height < grass_upper){
      pixel_colour = grass_tex;
   }
   else if(height > sand_line && height < grass_lower){
      pixel_colour = mix(sand_tex, grass_tex, (height-sand_line)/(grass_lower-sand_line));
   }
   else if(height < sand_line){
      pixel_colour = sand_tex;
   }

   float slope = length(frag_normal_transformed.xz);

   float rock_slope_start = 0.75;
   float rock_slope_end = 0.85;

   if(slope > rock_slope_start && slope < rock_slope_end){
      // float relative_slope = (slope-rock_slope_start)/(rock_slope_end-rock_slope_start);
      // pixel_colour = relative_slope*rock_tex+(1.0-relative_slope)*pixel_colour;
      pixel_colour = mix(pixel_colour, rock_tex, (slope-rock_slope_start)/(rock_slope_end-rock_slope_start));
      // pixel_colour = rock_tex;
   }
   else if(slope > rock_slope_end){
      pixel_colour = rock_tex;
   }

   shadow = get_shadow_val(0.0f, 0.0f);

   if(!activate_colour){
      pixel_colour = vec3(1.0);
   }

   if(activate_heightmap){
      color.rgb = vec3(frag_nontransfheight/2.0+0.5, frag_nontransfheight/2.0+0.5, frag_nontransfheight/2.0+0.5);
   }
   else{
      vec3 lighting_power = sun_light*sun_col;
      color.rgb = pixel_colour*lighting_power*shadow;
   }

   // we want black lines if wireframe is activated
   if (activate_wireframe){
      color.rgb = vec3(0.0);
   }

   color.a = 1.0;
}

float get_shadow_val(float offset_x, float offset_y){
   float shadow = 1.0;
   vec2 offset = vec2(offset_x, offset_y);

   float BIAS = 0.0001;

   vec2 shadow_texture_position = (shadow_coord.xy/shadow_coord.w)+offset;

   if(shadow_texture_position.x > 0.0 && shadow_texture_position.x < 1.0 && shadow_texture_position.y > 0.0 && shadow_texture_position.y < 1.0){
      if ( texture( shadow_buffer_tex, shadow_texture_position ).x  <  shadow_coord.z/shadow_coord.w-BIAS){
          shadow = 0.3;
      }
   }
   return shadow;
}
