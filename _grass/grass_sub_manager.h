#ifndef GRASS_SUB_MANAGER_H
#define GRASS_SUB_MANAGER_H

#include "../_terrain_quad/qterrain.h"

//sub section for grass
class Grass_sub_manager{

public:

   virtual void init(QTerrain *terrain, float water_height, GLuint _pid_grass, int nb_grass_side, float start_x, float end_x, float start_y, float end_y){
      srand(time(0));

      this->nb_grass_elem_side = nb_grass_side;

      //defines size of subsection
      this->start_x = start_x;
      this->end_x = end_x;
      this->start_y = start_y;
      this->end_y = end_y;

      _pid = _pid_grass;
      this->water_height = water_height;

      this->terrain = terrain;
   }

   virtual void load(){
      Noise_generator noise_gen_grass_prob;

      //add patches of grasses with a noise function
      noise_gen_grass_prob.set_noise_function(NOISE_SELECT_PERLIN);
      noise_gen_grass_prob.set_noise_level(3);

      {
         glGenVertexArrays(1, &_vao_grass);
         glBindVertexArray(_vao_grass);

         GLfloat vpoint[] = {
            -1.0f, 0.0f, 1.0f, // 0 bottom left
            1.0f, 0.0f, 1.0f, // 1 bottom right
            1.0f, 0.0f, -1.0f, // 2 top right
            -1.0f,  0.0f, -1.0f, // 3 top left
         };

         GLuint vpoint_index[] = {
            0, 1, 2,
            0, 2, 3
         };

         GLfloat per_surface_normals[] = {0, 1, 0,
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
         const GLfloat vtexcoord[] = { 0.0f, 1.0f,
                                       1.0f, 1.0f,
                                       1.0f, 0.0f,
                                       0.0f, 0.0f};

         GLuint _vbo_tex;
         glGenBuffers(1, &_vbo_tex);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
         glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

         GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
         glEnableVertexAttribArray(vtexcoord_id);
         glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      }

      nb_grass_to_draw = 0;

      Grass_element *ge = new Grass_element;
      ge->init(_pid, _vao_grass);

      for (size_t i = 0; i < nb_grass_elem_side; i++) {
         for (size_t j = 0; j < nb_grass_elem_side; j++) {
            //ge->init(pid_grass); //single vao per plane
            Transform t;
            float pos_x = start_x*(1-(float(i)/(nb_grass_elem_side-1)))+end_x*(float(i)/(nb_grass_elem_side-1));
            float pos_y = start_y*(1-(float(j)/(nb_grass_elem_side-1)))+end_y*(float(j)/(nb_grass_elem_side-1));

            int val_rand = rand()%1000;
            float frand = float(val_rand)/1000.0f;

            pos_x += (frand-0.5f) * 1.0f/(nb_grass_elem_side-1) * 1;
            pos_y += (frand-0.5f) * 1.0f/(nb_grass_elem_side-1) * 1;

            //the third parameter is the reduction in precision
            float grass_height = terrain->get_height(pos_x, pos_y, 3)+2.0;

            if(grass_height > water_height && grass_height < 30){
               float prob_grass = noise_gen_grass_prob.get_noise_val(pos_x/100, pos_y/100);
               if(prob_grass > frand+0.2){
                  t.translate(pos_x, grass_height+1.5, pos_y);
                  t.rotate(0.0, 1.0, 0.0, pos_x*pos_y*5.244);
                  t.scale(4.0, 4.0 + frand*4.0, 4.0);
                  ge->set_model_matrix(t.get_matrix());


                  mat_vector.push_back(ge->get_matrix_grass_0());
                  mat_vector.push_back(ge->get_matrix_grass_1());
               }//prob_grass > frand+0.2
            }
         }
      }
      //////////////////////////////// failed attempt to pass model transformation matrices
      //define the model transform in a vbo for the instanced rendering
      GLuint _vbo_transf;
      glGenBuffers(1, &_vbo_transf);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_transf);

      glm::mat4x4 *matrices_array = new glm::mat4x4[mat_vector.size()];

      printf("will draw %lu grass\n", mat_vector.size());

      for (size_t i = 0; i < mat_vector.size(); i++) {
         matrices_array[i] = mat_vector[i];
      }

      glBufferData(GL_ARRAY_BUFFER, mat_vector.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

      uint32_t vec4_size = sizeof(glm::vec4);

      //separate the matrix into 4
      GLuint model_attrib_0 = glGetAttribLocation(_pid, "model_mat");
      glEnableVertexAttribArray(model_attrib_0);
      glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

      glEnableVertexAttribArray(model_attrib_0+1);
      // uint32_t *offset = vec4_size;
      glVertexAttribPointer(model_attrib_0+1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

      glEnableVertexAttribArray(model_attrib_0+2);
      glVertexAttribPointer(model_attrib_0+2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2*vec4_size));

      glEnableVertexAttribArray(model_attrib_0+3);
      glVertexAttribPointer(model_attrib_0+3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));

      glVertexAttribDivisor(model_attrib_0, 1);
      glVertexAttribDivisor(model_attrib_0+1, 1);
      glVertexAttribDivisor(model_attrib_0+2, 1);
      glVertexAttribDivisor(model_attrib_0+3, 1);


      glBindVertexArray(0);

      nb_grass_to_draw = mat_vector.size();
      mat_vector.clear();
      delete[] matrices_array;

   }

   virtual void set_enabled(bool enabled){
      this->enabled = enabled;
   }

   virtual float get_start_x(){
      return start_x;
   }

   virtual float get_end_x(){
      return end_x;
   }

   virtual float get_start_y(){
      return start_y;
   }

   virtual float get_end_y(){
      return end_y;
   }

   void draw(){

      glBindVertexArray(_vao_grass);

      if(enabled){
         //instance
         glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, nb_grass_to_draw);

      }

      glBindVertexArray(0);
   }

protected:

   GLuint _vao_grass;
   GLuint _pid;
   bool enabled;

   QTerrain *terrain;
   uint nb_grass_elem_side;
   uint nb_grass_to_draw;

   float water_height;
   float start_x;
   float end_x;
   float start_y;
   float end_y;

   std::vector<glm::mat4x4> mat_vector;

};

#endif
