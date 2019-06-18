#version 330 core
uniform sampler2D tex;
uniform sampler2D ao_tex;
//uniform sampler2DMS tex;
uniform float tex_width;
uniform float tex_height;
in vec2 uv;
out vec4 color;

uniform uint effect_select;

float col_to_gs(vec3 vec){
   return 0.21*vec.x + 0.72*vec.y + 0.07*vec.z;
}

void main() {

   vec3 color_out = vec3(0.5, 1.0, 0.0);

   if(effect_select == 0u){ //normal
      color_out = texture(tex, uv).rgb;
   }
   else if(effect_select == 5u){ //sobel edge detection

      const int sobel_x_size = 3;
      float sobel_x[sobel_x_size*sobel_x_size] = float[](1, 0, -1, 2, 0, -2, 1, 0, -1);

      const int sobel_y_size = 3;
      float sobel_y[sobel_y_size*sobel_y_size] = float[](1, 2, 1, 0, 0, 0, -1, -2, -1);


      float edge_x = 0.0;
      float edge_y = 0.0;

      for(int i = 0; i < sobel_x_size; i++){
         float rel_i = i-(sobel_x_size-1.0)/2.0;

         for(int j = 0; j < sobel_y_size; j++){
            float rel_j = j-(sobel_y_size-1.0)/2.0;
            float grayscale_pixel = col_to_gs(texture(tex, uv+vec2(rel_i/tex_width,rel_j/tex_height)).rgb);
            //color_out += matrix_val*textureOffset(tex, uv, ivec2(rel_i, rel_j)).rgb; //textureoffset requires constants in the offset vector
            edge_x += sobel_x[j*3+i]*grayscale_pixel;
            edge_y += sobel_y[j*3+i]*grayscale_pixel;
         }
      }

      color_out = vec3(sqrt(edge_x*edge_x+edge_y*edge_y));
   }
   color = vec4(color_out, 1.0);
}
