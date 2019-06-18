#ifndef QTERRAIN_H
#define QTERRAIN_H

#include <vector>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../_terrain/terrain.h"
#include "../shader_helper.h"
#include "../transform.h"
#include "../noise_generator.hpp"
#include "../drawable.h"
#include "qtree.h"

#include "../texture.h"

//orders of sub elements:
// 0 1
// 2 3

class QTerrain : public Drawable{
public:
   QTerrain() : quad_tree_root(NULL, 2, 5, 0){

   }

   ~QTerrain(){
      quad_tree_root.delete_root();
   }

   void init(Noise_generator *noise_gen, float factor_distance_lod, unsigned int initial_granularity, unsigned int lod_max_levels, unsigned int noise_generator_max_level){
      _pid = load_shaders_files("terrain_vshader.glsl", "terrain_fshader.glsl");
      if(_pid == 0) exit(-1);

      this->noise_generator = *noise_gen;
      set_factor_distance_lod(factor_distance_lod);

      this->initial_granularity = initial_granularity;

      this->noise_generator_max_level = noise_generator_max_level;

      activate_colour = true;
      activate_heightmap = false;
      activate_wireframe = false;

      //give the root a noise generator at level 0 if it has ever to draw something, it should draw it with
      // simplest geometry
      // the qterrain itself keeps a noise generator at higher level (will be solicited fro cam position in fps mode)
      Noise_generator root_noise_gen(*noise_gen);

      //sets the noise level as the same as the log of the granularity (to avoid that the noise gen calculates more that it can display)
      uint noise_level = this->log2(initial_granularity);
      if (noise_level > noise_generator_max_level)
         noise_level = noise_generator_max_level;
      root_noise_gen.set_noise_level( noise_level );

      quad_tree_root.max_levels = lod_max_levels;
      quad_tree_root.subdivisions = initial_granularity;
      quad_tree_root.noise_generator_max_level = noise_generator_max_level;
      quad_tree_root.set_as_root(root_noise_gen, _pid);


      ////// use saved texture instead of generated ones
      // texture_grass.load_image("grass_texture.jpg");
      // glGenTextures(1, &_tex_grass);
      // glBindTexture(GL_TEXTURE_2D, _tex_grass);
      // //float tex_debug[] = {1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0};
      // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_grass.get_width(), texture_grass.get_height(), 0, GL_RGB, GL_FLOAT, texture_grass.get_tex_data());
      // //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, 0, GLRGB, GL_UNSIGNED_BYTE, data);
      // //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2,2, 0, GL_RGBA, GL_FLOAT, tex_debug);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glGenerateMipmap(GL_TEXTURE_2D);
      // GLuint tex_id = glGetUniformLocation(_pid, "tex_grass");
      // glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
      //
      //
      // texture_sand.load_image("sand_texture.jpg");
      // glGenTextures(1, &_tex_sand);
      // glBindTexture(GL_TEXTURE_2D, _tex_sand);
      // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_sand.get_width(), texture_sand.get_height(), 0, GL_RGB, GL_FLOAT, texture_sand.get_tex_data());
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glGenerateMipmap(GL_TEXTURE_2D);
      // tex_id = glGetUniformLocation(_pid, "tex_sand");
      // glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);
      //
      // texture_snow.load_image("snow_texture.jpg");
      // glGenTextures(1, &_tex_snow);
      // glBindTexture(GL_TEXTURE_2D, _tex_snow);
      // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_snow.get_width(), texture_snow.get_height(), 0, GL_RGB, GL_FLOAT, texture_snow.get_tex_data());
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glGenerateMipmap(GL_TEXTURE_2D);
      // tex_id = glGetUniformLocation(_pid, "tex_snow");
      // glUniform1i(tex_id, 2 /*GL_TEXTURE2*/);
      //
      // texture_rock.load_image("rock_texture.jpg");
      // glGenTextures(1, &_tex_rock);
      // glBindTexture(GL_TEXTURE_2D, _tex_rock);
      // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_rock.get_width(), texture_rock.get_height(), 0, GL_RGB, GL_FLOAT, texture_rock.get_tex_data());
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glGenerateMipmap(GL_TEXTURE_2D);
      // tex_id = glGetUniformLocation(_pid, "tex_rock");
      // glUniform1i(tex_id, 3 /*GL_TEXTURE3*/);
   }

   //draw all sub terrains
   void sub_draw(Quad_tree_node* node, glm::mat4x4 model, glm::mat4x4 view, glm::mat4x4 projection, GLfloat light_position[3], GLfloat camera_position[3], bool activate_colour, bool activate_heightmap, bool activate_wireframe){
      if(node->is_tail){
         node->terrain->draw(model, view, projection, light_position, camera_position, activate_colour, activate_heightmap, activate_wireframe);
      }
      else{
         for (unsigned int i = 0; i <= 3; i++){
            sub_draw(node->children[i], model, view, projection, light_position, camera_position, activate_colour, activate_heightmap, activate_wireframe);
         }
      }
   }

   void draw(){

      glUseProgram(_pid);
      glUniform3fv( glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid, "sun_col"), 1, this->sun_col);

      glUniform4fv( glGetUniformLocation(_pid, "clip_coord"), 1, this->clip_coord);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_grass);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_grass");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _tex_sand);
      tex_id = glGetUniformLocation(_pid, "tex_sand");
      glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, _tex_snow);
      tex_id = glGetUniformLocation(_pid, "tex_snow");
      glUniform1i(tex_id, 2 /*GL_TEXTURE2*/);

      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, _tex_rock);
      tex_id = glGetUniformLocation(_pid, "tex_rock");
      glUniform1i(tex_id, 3 /*GL_TEXTURE3*/);

      if(has_shadow_buffer){
         glUniformMatrix4fv( glGetUniformLocation(_pid, "shadow_matrix"), 1, GL_FALSE, glm::value_ptr(this->shadow_matrix));

         glUniform1i( glGetUniformLocation(_pid, "shadow_buffer_texture_width"), shadow_buffer_texture_width);
         glUniform1i( glGetUniformLocation(_pid, "shadow_buffer_texture_height"), shadow_buffer_texture_height);

         glActiveTexture(GL_TEXTURE4);
         glBindTexture(GL_TEXTURE_2D, _shadow_texture_id);
         GLuint tex_id = glGetUniformLocation(_pid, "shadow_buffer_tex");
         glUniform1i(tex_id, 4 /*GL_TEXTURE4*/);
      }

      glUseProgram(0);

      sub_draw(&quad_tree_root, model_matrix, view_matrix, projection_matrix, light_position, camera_position, activate_colour, activate_heightmap, activate_wireframe);
   }

   void set_colour(bool activate){
      activate_colour = activate;
   }

   void set_heightmap(bool activate){
      activate_heightmap = activate;
   }

   void set_wireframe(bool activate){
      activate_wireframe = activate;
   }

   //update the precision of the terrain depending on the position of camera
   void update_lod_camera(GLfloat camera_position[3]){
      update_lod_camera_recurs(&quad_tree_root, camera_position);
   }

   //get height at a given point (takes into account matrix)
   float get_height(float pos_x, float pos_y, int precision_reduction = 0){
      //invert the model matrix on the positions in order to be in the camera space
      glm::vec4 vec(pos_x, 1.0f, pos_y, 1.0f);
      glm::mat4 transf_matrix = model_matrix;
      glm::vec4 vec_transf = glm::inverse(transf_matrix)*vec;

      //since every sub terrains uses the same noise generatior, not really a need to find the specific
      //sub terrain to get its height, use the root one

      int current_level = noise_generator.get_noise_level();
      noise_generator.set_noise_level(current_level-precision_reduction);
      float height = noise_generator.get_noise_val(vec_transf[0], vec_transf[2]);
      noise_generator.set_noise_level(current_level);

      height = (transf_matrix*glm::vec4(0.0f, height, 0.0f, 1.0f))[1];

      return height;
   }

   void set_factor_distance_lod(float factor_distance){
      assert(factor_distance > 0.0f);
      this->factor_distance_lod = factor_distance;
   }

protected:
   Quad_tree_node quad_tree_root;

   Noise_generator noise_generator;

   float factor_distance_lod;

   //sets the maximum granularity for terrains when they are at level 0 (i.e how many division in the subterrain)
   //if the value is equals to two then, terrain at level 0 will be 2x2 squares
   unsigned int initial_granularity;

   //max levels that the lod will go, higher == more granularity but more computation
   //0 = only root terrain will be displayed
   unsigned int lod_max_levels;

   unsigned int noise_generator_max_level;

   bool activate_colour;
   bool activate_wireframe;
   bool activate_heightmap;

   GLuint _tex_grass;
   Texture texture_grass;

   GLuint _tex_sand;
   Texture texture_sand;

   GLuint _tex_snow;
   Texture texture_snow;

   GLuint _tex_rock;
   Texture texture_rock;

   //update the camera for all terrains in the qtree
   void update_lod_camera_recurs(Quad_tree_node *qnode, GLfloat camera_pos[3]){
      float centre[3];
      centre[0] = (qnode->start_x+qnode->end_x)/2.0;//x
      centre[1] = 0.0f;
      centre[2] = (qnode->start_y+qnode->end_y)/2.0;//z

      glm::vec4 centre_v4 = glm::vec4(centre[0], centre[1], centre[2], 1.0);//model_transf.get_matrix();
      centre_v4 = model_matrix*centre_v4;

      float squared_distances[0];
      squared_distances[0] = (centre_v4[0]-camera_pos[0])*(centre_v4[0]-camera_pos[0]);
      squared_distances[1] = (centre_v4[1]-camera_pos[1])*(centre_v4[1]-camera_pos[1]);
      squared_distances[2] = (centre_v4[2]-camera_pos[2])*(centre_v4[2]-camera_pos[2]);

      float distance = sqrt(squared_distances[0]+squared_distances[2]);
      distance *= factor_distance_lod;
      float resize_mat = sqrt(model_matrix[0][0]*model_matrix[2][2])*3;

      if (!qnode->is_tail){ //not tail, check if need to merge, otherwise pass to children
         if(distance > (qnode->end_x-qnode->start_x)*resize_mat){
            qnode->merge_qtree();
         }
         else{
            for (unsigned int i = 0; i <= 3; i++){
               update_lod_camera_recurs(qnode->children[i], camera_pos);
            }
         }
      }
      else{ //is tail, check if need to break
         if(distance < (qnode->end_x-qnode->start_x)*resize_mat){
            qnode->break_qtree();
         }
      }
   }

   //to get height of a specific sub element.
   float get_height_norm_recurs(float pos_x, float pos_y, Quad_tree_node *node){
      if(node->is_tail){
         return node->terrain->get_height(pos_x, pos_y);
      }
      else{
         for(unsigned int i = 0; i < 3; i++){
            Quad_tree_node *child = node->children[i];
            if(pos_x > child->start_x && pos_x < child->end_x && pos_y > child->start_y && pos_y < child->end_y){
               get_height_norm_recurs(pos_x, pos_y, child);
            }
         }
      }
   }

   unsigned int log2(unsigned int val){
      unsigned int ret_val = 0;

      while (val >>=1 != 0){
         ret_val++;
      }

      return ret_val;
   }
};

#endif
