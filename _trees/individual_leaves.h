#ifndef INDIVIDUAL_LEAVES_H
#define INDIVIDUAL_LEAVES_H

#include "../drawable.h"
#include "../transform.h"

class Individual_leaves : public Drawable{
public:
   void init(){
      GLuint pid = load_shaders_files("leaves_individual_vshader.glsl", "leaves_individual_fshader.glsl");
      srand(time(0));

      init(pid);
   }

   void init(GLuint pid){
      glGenVertexArrays(1, &_vao);
      init(pid, _vao);
   }

   void init(GLuint pid, GLuint _vao){

      this->_pid = pid;
      if(_pid == 0) exit(-1);

      glBindVertexArray(_vao);

      GLfloat vpoint[] = {
         -1.0f, -1.0f, 0.0f, // 0 bottom left
         1.0f, -1.0f, 0.0f, // 1 bottom right
         0.0f, 1.0f, 0.0f, // 2 top
      };

      GLuint vpoint_index[] = {
         0, 1, 2,
      };

      GLfloat per_surface_normals[] = {0, 1, 0,
         0, 1, 0,
         0, 1, 0,
      };

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

      GLuint vpoint_id = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "surface_normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);

      //texture coord definition
      const GLfloat vtexcoord[] = { 0.0f, 1.0f,
                                    1.0f, 1.0f,
                                    0.5f, 0.0f};

      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindVertexArray(0);

   }

   //generate the leaves in a sphere around top of tree
   void generate(){
      mat_vector.clear();
      pos_vector.clear();
      float incr = 0.12f;
      for (float x = -1.0; x < 1.0f; x+=incr) {
         for (float y = -1.0; y < 1.0f; y+=incr) {
            for (float z = -1.0; z < 1.0f; z+=incr) {
               float dist_to_centre = sqrt(x*x+y*y+z*z);
               if (dist_to_centre < 1.0f){

                  int val_rand = rand()%1000;
                  float frand = float(val_rand)/1000.0f;
                  Transform t;

                  Transform random_rot;
                  random_rot.rotate(1.0, 0.0, 0.0, frand);
                  random_rot.rotate(0.0, 1.0, 0.0, frand);
                  random_rot.rotate(0.0, 0.0, 1.0, frand);

                  t.mult(random_rot);

                  //basically a scale of the position
                  t.translate(x*9.0, y*9.0, z*9.0);

                  t.mult(random_rot); //rotates again the leaves so they don't have the same orientation
                  //size of individual leaves
                  float leaves_scale = 0.4+(1.0f-dist_to_centre);
                  t.scale(leaves_scale, leaves_scale, leaves_scale);

                  //push this matrix in a vector, will be fed to the shader buffer
                  mat_vector.push_back(this->model_matrix*t.get_matrix());
                  glm::vec4 val(x, y, z, 1);
                  glm::vec4 val_transf = random_rot.get_matrix()*val;
                  glm::vec3 val_transf3(val_transf.x, val_transf.y, val_transf.z);
                  pos_vector.push_back(val_transf3);
               }
            }
         }
      }
   }

   void build_matrices(){
      GLuint _vbo_transf;
      glGenBuffers(1, &_vbo_transf);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_transf);

      glm::mat4x4 *matrices_array = new glm::mat4x4[mat_vector.size()];

      printf("will draw %lu leaves\n", mat_vector.size());

      for (size_t i = 0; i < mat_vector.size(); i++) {
         matrices_array[i] = mat_vector[i];
      }

      glBufferData(GL_ARRAY_BUFFER, mat_vector.size()*sizeof(glm::mat4x4), matrices_array, GL_STATIC_DRAW);

      uint32_t vec4_size = sizeof(glm::vec4);

      //separate model matrix into four rows
      GLuint model_attrib_0 = glGetAttribLocation(_pid, "model_mat");
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

      nb_leaves_to_draw = mat_vector.size();
      mat_vector.clear();
      delete[] matrices_array;
   }

   uint get_nb_leaves_to_draw(){
      return nb_leaves_to_draw;
   }

   std::vector<glm::mat4x4> get_mat_vector(){
      return mat_vector;
   }

   std::vector<glm::vec3> get_pos_vector(){
      return pos_vector;
   }

   void draw(){

      glBindVertexArray(_vao);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix*transf.get_matrix()));

      if(enabled){
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }

      glBindVertexArray(0);
   }
protected:

   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_sur_norm;
   GLuint _vbo_idx;
   GLuint _vbo_tex;
   GLuint _pid;

   Transform transf;

   uint nb_leaves_to_draw;

   std::vector<glm::mat4x4> mat_vector;
   std::vector<glm::vec3> pos_vector;
};

#endif
