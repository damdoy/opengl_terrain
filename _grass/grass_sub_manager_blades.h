#ifndef GRASS_SUB_MANAGER_BLADES_H
#define GRASS_SUB_MANAGER_BLADES_H

#include <vector>
#include <cmath>

#include "grass_element.h"
#include "../noise_generator.hpp"
#include "../texture.h"
#include "../_terrain_quad/qterrain.h"

class Grass_sub_manager_blades : public Grass_sub_manager{

public:

   void init(QTerrain *terrain, float water_height, GLuint _pid_grass, int nb_grass_side, float start_x, float end_x, float start_y, float end_y){
      srand(time(0));

      Grass_sub_manager::init(terrain, water_height, _pid_grass, nb_grass_side, start_x, end_x, start_y, end_y);

   }

   virtual void load(){

      Noise_generator noise_gen_grass_prob;

      //noise function, to be used to create patches of grass, but doesn't look that good
      noise_gen_grass_prob.set_noise_function(NOISE_SELECT_PERLIN);
      noise_gen_grass_prob.set_noise_level(1);

      {
         unsigned int err = 0;
         err = glGetError();
         if(err != 0){
            printf("1ERROR OGL: %d\n", err);
         }
      }

      vao_grass = 0;

      {
         glGenVertexArrays(1, &vao_grass);
         glBindVertexArray(vao_grass);

         //position of points on the grass blade
         //  0
         //
         // 1.2
         // 3 4
         // 5 6

         GLfloat vpoint[] = {
            0.0f, 0.0f, -2.0f, // 0
            -1.0f, 0.0f, 0.0f, // 1
            1.0f, 0.0f, 0.0f, // 2
            -1.0f,  0.0f, 1.0f, // 3
            1.0f, 0.0, 1.0f, // 4
            -1.0f, 0.0, 2.0f, // 5
            1.0f, 0.0, 2.0f, // 6
         };

         GLuint vpoint_index[] = {
            0, 1, 2,
            2, 1, 3,
            2, 3, 4,
            4, 3, 5,
            4, 5, 6,
         };

         GLfloat per_surface_normals[] = {0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
            0, 1, 0,
         };

         GLuint _vbo;

         glGenBuffers(1, &_vbo);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo);
         glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

         GLuint vpoint_id = glGetAttribLocation(_pid, "position");
         glEnableVertexAttribArray(vpoint_id);
         glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

         GLuint _vbo_sur_norm;
         glGenBuffers(1, &_vbo_sur_norm);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
         glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

         GLuint id_pos = glGetAttribLocation(_pid, "surface_normal");
         glEnableVertexAttribArray(id_pos);
         glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

         GLuint _vbo_idx;
         glGenBuffers(1, &_vbo_idx);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
         glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);

         //texture coord definition
         const GLfloat vtexcoord[] = { 0.5f, 0.0f, // 0
                                       0.0f, 0.5f, // 1
                                       1.0f, 0.5f, // 2
                                       0.0f, 0.75f, // 3
                                       1.0f, 0.75f, // 4
                                       0.0f, 1.0f, // 5
                                       1.0f, 1.0f}; // 6

         GLuint _vbo_tex;
         glGenBuffers(1, &_vbo_tex);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
         glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

         // {
         //    //seems to always be thrown
         //    unsigned int err = 0;
         //    err = glGetError();
         //    if(err != 0){
         //       printf("2ERROR OGL: %d\n", err);
         //    }
         // }

         GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
         glEnableVertexAttribArray(vtexcoord_id);
         glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      }

      for (size_t i = 0; i < nb_grass_elem_side; i++) {
         for (size_t j = 0; j < nb_grass_elem_side; j++) {
            Grass_element ge;
            ge.init_matrices(); //load the matrices
            Transform t;

            float pos_x = start_x*(1-(float(i)/(nb_grass_elem_side-1)))+end_x*(float(i)/(nb_grass_elem_side-1));
            float pos_y = start_y*(1-(float(j)/(nb_grass_elem_side-1)))+end_y*(float(j)/(nb_grass_elem_side-1));

            int val_rand = rand()%1000;
            float frand = float(val_rand)/1000.0f;
            pos_x += (frand-0.5f)/1.0f;

            val_rand = rand()%1000;
            frand = float(val_rand)/1000.0f;
            pos_y += (frand-0.5f)/1.0f;

            val_rand = rand()%1000;
            frand = float(val_rand)/1000.0f;
            float grass_height = terrain->get_height(pos_x, pos_y, 2)+2.0;

            float variation = sin(cos(pos_y*0.1)+pos_x*0.3)*2; //low freq variations for limit grass in hight

            // only draw grass if at a reasonable height (not in water or snow)
            if(grass_height > water_height+variation && grass_height < 35+variation){
                  t.translate(pos_x, grass_height, pos_y);
                  t.rotate(0.0, 1.0, 0.0, pos_x*pos_y*5.244);
                  t.scale(0.3, 1.0, 0.3);
                  t.scale(1.0, 1.0 + frand*0.5-0.25, 1.0);
                  t.translate(0, (1.0 + (frand*0.5-0.25))/2.0f, 0);
                  ge.set_model_matrix(t.get_matrix());

                  //only add one transf, as cull_face will be deactivated during draw
                  mat_vector.push_back(ge.get_matrix_grass_0());
            }
         }
      }

      unsigned int err = 0;
      err = glGetError();
      if(err != 0){
         printf("3ERROR OGL: %d\n", err);
      }

      GLuint _vbo_transf;
      glGenBuffers(1, &_vbo_transf);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_transf);

      glm::mat4x4 *matrices_array = new glm::mat4x4[mat_vector.size()];

      printf("will draw %lu grass\n", mat_vector.size());
      nb_grass_to_draw = mat_vector.size();

      for (size_t i = 0; i < mat_vector.size(); i++) {
         matrices_array[i] = mat_vector[i];
      }
      {
         unsigned int err = 0;
         err = glGetError();
         if(err != 0){
            printf("4ERROR OGL: %d\n", err);
         }
      }
      glBufferData(GL_ARRAY_BUFFER, mat_vector.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

      uint32_t vec4_size = sizeof(glm::vec4);

      //separate the in 4 to be given as attrib
      GLuint model_attrib_0 = glGetAttribLocation(_pid, "model_mat");
      printf("model_attrib:%d\n", model_attrib_0);

      {
         unsigned int err = 0;
         err = glGetError();
         if(err != 0){
            printf("44ERROR OGL: %d\n", err);
         }
      }
      // GLuint model_attrib_0 = 10;
      glEnableVertexAttribArray(model_attrib_0);
      glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

      glEnableVertexAttribArray(model_attrib_0+1);
      glVertexAttribPointer(model_attrib_0+1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

      glEnableVertexAttribArray(model_attrib_0+2);
      glVertexAttribPointer(model_attrib_0+2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2*vec4_size));

      glEnableVertexAttribArray(model_attrib_0+3);
      glVertexAttribPointer(model_attrib_0+3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));
      {
         unsigned int err = 0;
         err = glGetError();
         if(err != 0){
            printf("444ERROR OGL: %d\n", err);
         }
      }

      glVertexAttribDivisor(model_attrib_0, 1);
      glVertexAttribDivisor(model_attrib_0+1, 1);
      glVertexAttribDivisor(model_attrib_0+2, 1);
      glVertexAttribDivisor(model_attrib_0+3, 1);

      err = glGetError();
      if(err != 0){
         printf("5ERROR OGL: %d\n", err);
      }


      glBindVertexArray(0);

      nb_grass_to_draw = mat_vector.size();
      mat_vector.clear();
      delete[] matrices_array;
   }

   virtual void set_enabled(bool enabled){
      this->enabled = enabled;
   }

   void set_light_pos(GLfloat light_position[3]){
      this->light_position[0] = light_position[0];
      this->light_position[1] = light_position[1];
      this->light_position[2] = light_position[2];
   }

   void set_camera_pos(GLfloat camera_position[3]){
      this->camera_position[0] = camera_position[0];
      this->camera_position[1] = camera_position[1];
      this->camera_position[2] = camera_position[2];
   }

   virtual void set_view_matrix(glm::mat4x4 view){
      this->view_matrix = view;
   }

   virtual void set_projection_matrix(glm::mat4x4 projection){
      this->projection_matrix = projection;
   }

   void draw(){

      if(enabled)
      {
         glBindVertexArray(vao_grass);

         glDisable(GL_CULL_FACE);

         glDrawElementsInstanced(GL_TRIANGLES, 15, GL_UNSIGNED_INT, 0, nb_grass_to_draw);

         glEnable(GL_CULL_FACE);
         glBindVertexArray(0);
      }
   }

protected:

   uint nb_grass_to_draw;

   float camera_position[3];
   float light_position[3];

   glm::mat4x4 view_matrix;
   glm::mat4x4 projection_matrix;

   const float size_to_draw = 100;

   GLuint vao_grass;

   std::vector<glm::mat4x4> mat_vector;
};

#endif
