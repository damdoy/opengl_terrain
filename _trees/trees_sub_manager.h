#ifndef TREES_SUB_MANAGER_H
#define TREES_SUB_MANAGER_H

#include "../drawable.h"
#include "trunk.h"
#include "../_terrain_quad/qterrain.h"
#include "individual_leaves.h"

class Trees_sub_manager{

public:

   void init(QTerrain *terrain, float water_height, GLuint _pid_trunk, GLuint _pid_ileaves, int nb_trees_side, float start_x, float end_x, float start_y, float end_y){
      srand(time(0));

      enabled = true;

      this->nb_trees_elem_side = nb_trees_side;

      this->start_x = start_x;
      this->end_x = end_x;
      this->start_y = start_y;
      this->end_y = end_y;

      this->_pid_trunk = _pid_trunk;
      this->_pid_ileaves = _pid_ileaves;
      this->water_height = water_height;

      this->terrain = terrain;

   }

   virtual void load(){
      Noise_generator noise_gen_tree_prob;

      noise_gen_tree_prob.set_noise_function(NOISE_SELECT_PERLIN);
      noise_gen_tree_prob.set_noise_level(3);


      float increment_x = (end_x - start_x)/nb_trees_elem_side;
      float increment_y = (end_y - start_y)/nb_trees_elem_side;

      uint count_tree = 0;

      srand(0);

      glGenVertexArrays(1, &_vao_ileaves);
      Individual_leaves ileaves;
      ileaves.init(_pid_ileaves, _vao_ileaves);

      glGenVertexArrays(1, &_vao_trunk);
      Trunk trunk;
      trunk.init(_pid_trunk, _vao_trunk);

      for (size_t i = 0; i < nb_trees_elem_side; i++) {
         for (size_t j = 0; j < nb_trees_elem_side; j++) {
            Transform transf;

            int val_rand = rand()%1000;
            float frand = float(val_rand)/1000.0f;
            frand -= 0.5f;

            float pos_x = start_x + i*increment_x + frand*increment_x;
            float pos_z = start_y + j*increment_y + frand*increment_y;
            float tree_height = terrain->get_height(pos_x, pos_z);
            if(tree_height > water_height && tree_height < 30){ //only put tree above water
               float prob_tree = noise_gen_tree_prob.get_noise_val(pos_x/100, pos_z/100);
               if(prob_tree > (frand+0.5f)+0.2f){ //don't put trees everywhere, group them with perlin noise
                  //make a model matrix for the tree
                  transf.translate(pos_x, tree_height, pos_z);
                  transf.rotate(0, 1, 0, frand*10.0f);

                  trunk.set_model_matrix(transf.get_matrix());

                  transf.translate(0, 16, 0);
                  transf.scale(1.0, 0.5, 1.0);

                  //move the leaves according to the tree matrix
                  ileaves.set_model_matrix(transf.get_matrix());
                  ileaves.generate();

                  std::vector<glm::mat4x4> ileaves_mat = ileaves.get_mat_vector();
                  std::vector<glm::vec3> ileaves_pos_mat = ileaves.get_pos_vector();

                  // insert the model vector of leaves into the main vector
                  mat_vector_ileaves.insert(mat_vector_ileaves.end(), ileaves_mat.begin(), ileaves_mat.end());
                  pos_vector_ileaves.insert(pos_vector_ileaves.end(), ileaves_pos_mat.begin(), ileaves_pos_mat.end());

                  std::vector<glm::mat4> trunks = trunk.get_transf();
                  for (size_t i = 0; i < trunks.size(); i++) {
                     mat_vector_trunks.push_back(trunks[i]);
                  }
                  count_tree++;
               }
            }
         }
      }

      trunk_indices_to_draw = trunk.indices_to_draw();

      //LEAVES
      {
         glBindVertexArray(_vao_ileaves);

         GLuint _vbo_transf;
         glGenBuffers(1, &_vbo_transf);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_transf);

         glm::mat4x4 *matrices_array = new glm::mat4x4[mat_vector_ileaves.size()];

         printf("will draw %lu individual leaves\n", mat_vector_ileaves.size());

         for (size_t i = 0; i < mat_vector_ileaves.size(); i++) {
            matrices_array[i] = mat_vector_ileaves[i];
         }

         glBufferData(GL_ARRAY_BUFFER, mat_vector_ileaves.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

         uint32_t vec4_size = sizeof(glm::vec4);

         GLuint model_attrib_0 = glGetAttribLocation(_pid_ileaves, "model_mat");
         glEnableVertexAttribArray(model_attrib_0);
         glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

         glEnableVertexAttribArray(model_attrib_0+1);
         glVertexAttribPointer(model_attrib_0+1, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(vec4_size));

         glEnableVertexAttribArray(model_attrib_0+2);
         glVertexAttribPointer(model_attrib_0+2, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(2*vec4_size));

         glEnableVertexAttribArray(model_attrib_0+3);
         glVertexAttribPointer(model_attrib_0+3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(uintptr_t)(3 * vec4_size));

         glVertexAttribDivisor(model_attrib_0, 1);
         glVertexAttribDivisor(model_attrib_0+1, 1);
         glVertexAttribDivisor(model_attrib_0+2, 1);
         glVertexAttribDivisor(model_attrib_0+3, 1);

         //draw the raw position of the leaves
         GLuint _vbo_base_pos;
         glGenBuffers(1, &_vbo_base_pos);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_base_pos);

         glm::vec3 *pos_array = new glm::vec3[pos_vector_ileaves.size()];

         printf("will draw %lu individual leaves positions\n", pos_vector_ileaves.size());

         for (size_t i = 0; i < pos_vector_ileaves.size(); i++) {
            pos_array[i] = pos_vector_ileaves[i];
         }

         glBufferData(GL_ARRAY_BUFFER, pos_vector_ileaves.size()*sizeof(glm::vec3), pos_array, GL_STATIC_DRAW);

         uint32_t vec3_size = sizeof(glm::vec3);

         //gives the position of the leave
         GLuint pos_attrib = glGetAttribLocation(_pid_ileaves, "raw_position");
         glEnableVertexAttribArray(pos_attrib);
         glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 1 * vec3_size, (void*)0);

         glVertexAttribDivisor(pos_attrib, 1);

         glBindVertexArray(0);

         nb_ileaves_to_draw = mat_vector_ileaves.size();
         mat_vector_ileaves.clear();
         delete[] matrices_array;
      }

      ////////////////trunk
      {
         glBindVertexArray(_vao_trunk);
         GLuint _vbo_transf;
         glGenBuffers(1, &_vbo_transf);
         glBindBuffer(GL_ARRAY_BUFFER, _vbo_transf);

         glm::mat4 *matrices_array = new glm::mat4x4[mat_vector_trunks.size()];

         printf("will draw %lu trunks\n", mat_vector_trunks.size());

         for (size_t i = 0; i < mat_vector_trunks.size(); i++) {
            matrices_array[i] = mat_vector_trunks[i];
         }

         glBufferData(GL_ARRAY_BUFFER, mat_vector_trunks.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

         uint32_t vec4_size = sizeof(glm::vec4);

         GLuint model_attrib_0 = glGetAttribLocation(_pid_trunk, "model_mat");

         glEnableVertexAttribArray(model_attrib_0);
         glVertexAttribPointer(model_attrib_0, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0);

         glEnableVertexAttribArray(model_attrib_0+1);
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

         nb_trunks_to_draw = mat_vector_trunks.size();
         mat_vector_trunks.clear();
         delete[] matrices_array;
      }
   }

   virtual void set_enabled(bool enabled){
      this->enabled = enabled;
   }

   float get_start_x(){
      return start_x;
   }

   float get_end_x(){
      return end_x;
   }

   float get_start_y(){
      return start_y;
   }

   float get_end_y(){
      return end_y;
   }

   void draw_trunks(){

      glBindVertexArray(_vao_trunk);

      glUniformMatrix4fv( glGetUniformLocation(_pid_trunk, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid_trunk, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDisable(GL_CULL_FACE);
      if(enabled){
         glDrawElementsInstanced(GL_TRIANGLES, trunk_indices_to_draw, GL_UNSIGNED_INT, 0, nb_trunks_to_draw);
      }
      glEnable(GL_CULL_FACE);

      glBindVertexArray(0);
   }

   void draw_ileaves(){

      glBindVertexArray(_vao_ileaves);

      glUniformMatrix4fv( glGetUniformLocation(_pid_ileaves, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid_ileaves, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      if(enabled){
         glDisable(GL_CULL_FACE);
         glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0, nb_ileaves_to_draw);
         glEnable(GL_CULL_FACE);
      }

      glBindVertexArray(0);
   }

   void set_view_matrix(glm::mat4x4 view){
      for (size_t i = 0; i < lst_trunks.size(); i++) {
         lst_trunks[i].set_view_matrix(view);
      }
      this->view_matrix = view;
   }

   void set_projection_matrix(glm::mat4x4 projection){
      for (size_t i = 0; i < lst_trunks.size(); i++) {
         lst_trunks[i].set_projection_matrix(projection);
      }
      this->projection_matrix = projection;
   }

protected:

   GLuint _vao_grass;
   GLuint _pid_trunk;
   GLuint _vao_trunk;

   GLuint _pid_ileaves;
   GLuint _vao_ileaves;

   bool enabled;

   QTerrain *terrain;
   uint nb_trees_elem_side;
   uint nb_ileaves_to_draw;
   uint nb_trunks_to_draw;

   float water_height;
   float start_x;
   float end_x;
   float start_y;
   float end_y;

   uint trunk_indices_to_draw;

   glm::mat4x4 model_keep_debug;

   std::vector<glm::mat4x4> mat_vector_leaves;
   std::vector<glm::mat4x4> mat_vector_trunks;
   std::vector<glm::mat4x4> mat_vector_ileaves;
   std::vector<glm::vec3> pos_vector_ileaves;

   std::vector<Transform> lst_transf;
   std::vector<Trunk> lst_trunks;

   glm::mat4x4 view_matrix;
   glm::mat4x4 projection_matrix;

};

#endif
