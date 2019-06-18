#ifndef TRUNK_H
#define TRUNK_H

#include "../drawable.h"
#include "../transform.h"

//a cylinder with recursive sticks branching out
class Trunk : public Drawable{
public:

   virtual void init(){
      _pid = load_shaders_files("trunk_vshader.glsl", "trunk_fshader.glsl");
      if(_pid == 0) exit(-1);
      init(_pid);
   }

   void init(GLuint pid){
      glGenVertexArrays(1, &_vao);
      init(pid, _vao);
   }

   void create_positions(){

      const uint nb_vertices = 6;

      nb_positions = 3*nb_vertices*2+2*3;
      positions = new GLfloat[nb_positions];
      uint index = 0;

      positions[index] = 0;
      positions[index+1] = -1.0f/2.0f;
      positions[index+2] = 0;
      index+=3;
      positions[index] = 0;
      positions[index+1] = 1.0f/2.0f;
      positions[index+2] = 0;
      index+=3;

      const float bigger_base = 1.33; //to make base of trunk slightly larger

      for (size_t i = 0; i < nb_vertices; i++) {
         float angle_val = 3.1415f*2.0f*(1.0f/nb_vertices)*i;
         positions[index] = (sin(angle_val)/2.0f)*bigger_base;
         positions[index+1] = -1.0f/2.0f;
         positions[index+2] = (cos(angle_val)/2.0f)*bigger_base;
         index+=3;
         positions[index] = (sin(angle_val)/2.0f)*(1/bigger_base);
         positions[index+1] = 1.0f/2.0f;
         positions[index+2] = (cos(angle_val)/2.0f)*(1/bigger_base);
         index+=3;
      }

      nb_indices = 3*nb_vertices*2+3*nb_vertices*2;
      indices = new GLuint[nb_indices]; //3*2 per face, plus 3 per face for top and bottom

      index = 0;
      for (size_t i = 0; i < nb_vertices; i++) {
         uint start_idx_face = 2+2*i; // 12 points per face
         uint up_left = start_idx_face+1; //may be another one
         uint up_right = start_idx_face+3;
         if(up_right >= 2+2*nb_vertices){
            up_right = 2+1;
         }
         uint down_left = start_idx_face+0;
         uint down_right = start_idx_face+2;
         if(down_right >= 2+2*nb_vertices){
            down_right = 2;
         }

         //top
         indices[index] = 1;
         indices[index+1] = up_left;
         indices[index+2] = up_right;
         index+=3;
         //side
         indices[index] = up_left;
         indices[index+1] = down_left;
         indices[index+2] = down_right;
         index+=3;
         indices[index] = up_right;
         indices[index+1] = up_left;
         indices[index+2] = down_right;
         index+=3;
         //bottom
         indices[index] = 0;
         indices[index+1] = down_right;
         indices[index+2] = down_left;
         index+=3;
      }

      //per vertex textcoord
      nb_text_coord = 2*nb_vertices*2+2*2;
      text_coord = new GLfloat[nb_text_coord];

      index = 0;

      //dont care the top and bottom?
      text_coord[index] = 1;
      text_coord[index+1] = 1;
      index+=2;
      text_coord[index] = 0;
      text_coord[index+1] = 0;
      index+=2;

      for (size_t i = 0; i < nb_vertices; i++) {
         // i: 0 -> 0.5 == 0 -> 1
         // i: 0.5 -> 1.0 == 1 -> 0

         float relative_pos = float(i)/float(nb_vertices);
         if ( relative_pos < 0.5f){
            relative_pos = relative_pos*2;
         }
         else{ //relative_pos > 0.5f (0.5=1, 1.0=0)
            relative_pos = relative_pos*(-2)+2;
         }

         text_coord[index] = relative_pos; //bottom
         text_coord[index+1] = 1.0f;
         index+=2;
         text_coord[index] = relative_pos; //top
         text_coord[index+1] = 0.0f;
         index+=2;
      }

      //per vertex normal
      nb_normals = 3*nb_vertices*2+2*3;
      normals = new GLfloat[nb_normals];

      index = 0;

      normals[index] = 0.0f;
      normals[index+1] = -1.0f;
      normals[index+2] = 0.0f;
      index+=3;
      normals[index] = 0.0f;
      normals[index+1] = 1.0f;
      normals[index+2] = 0.0f;
      index+=3;

      for (size_t i = 0; i < nb_vertices; i++) {
         float angle_val = 3.1415f*2.0f*(1.0f/nb_vertices)*i;
         normals[index] = sin(angle_val);
         normals[index+1] = 0.0f;
         normals[index+2] = cos(angle_val);
         index+=3;
         normals[index] = sin(angle_val);
         normals[index+1] = 0.0f;
         normals[index+2] = cos(angle_val);
         index+=3;
      }
   }

   void init(GLuint pid, GLuint _vao){

      create_positions();

      this->_pid = pid;
      // std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

      glUseProgram(_pid);

      glBindVertexArray(_vao);

      glGenBuffers(1, &_vbo_pos);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_pos);
      glBufferData(GL_ARRAY_BUFFER, nb_positions*sizeof(GLfloat), positions, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, nb_normals*sizeof(GLfloat), normals, GL_STATIC_DRAW);

      id_pos = glGetAttribLocation(_pid, "normals");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, nb_indices*sizeof(GLuint), indices, GL_STATIC_DRAW);
      // printf("%d, %d\n", nb_positions*sizeof(GLfloat), nb_indices*sizeof(GLuint));

      GLuint _vbo_tex;
      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, nb_text_coord*sizeof(GLfloat), text_coord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindVertexArray(0);

      //main trunk
      Transform t;
      t.translate(0,7,0);
      t.scale(1.5, 14, 1.5);
      transf.push_back(t);

      add_sub_trunks(t, 0);
   }

   void add_sub_trunks(Transform t, uint level){

      //be able to do this transofrm
      //[[1.0, -0.1, 0.0], [0.0, 1.0, 0.0], [0.0, 0.1, 1.0]]

      // [x, 1.0, z] x-0.1, 1.0, z-0.1

      if(level >= 3){
         return;
      }

      Transform up;
      up.translate(0, 14, 0);
      up.scale(0.5, 0.25, 0.5);
      up.mult(t);
      transf.push_back(up);

      Transform s1;
      s1.translate(0.3, 7+3.5, 0);
      s1.rotate(0.0f, 0.0f, 1.0f, -3.1415f/4.0f);
      s1.scale(0.5, 0.5, 0.5);
      s1.mult(t);
      transf.push_back(s1);

      Transform s2;
      s2.translate(-0.3, 7+3.5, 0);
      s2.rotate(0.0f, 0.0f, 1.0f, 3.1415f/4.0f);
      s2.scale(0.5, 0.5, 0.5);
      s2.mult(t);
      transf.push_back(s2);

      Transform s3;
      s3.translate(0, 7+3.5, -0.0);
      s3.rotate(1.0f, 0.0f, 0.0f, 3.1415f/4.0f);
      s3.scale(0.5, 0.5, 0.5);
      s3.mult(t);
      transf.push_back(s3);

      Transform s4;
      s4.translate(0, 7+3.5, 0.0);
      s4.rotate(1.0f, 0.0f, 0.0f, -3.1415f/4.0f);
      s4.scale(0.5, 0.5, 0.5);
      s4.mult(t);
      transf.push_back(s4);

      add_sub_trunks(up, level+1);
      add_sub_trunks(s1, level+1);
      add_sub_trunks(s2, level+1);
      add_sub_trunks(s3, level+1);
      add_sub_trunks(s4, level+1);
   }

   //will be added to a drawing list instance by the sub manager
   std::vector<glm::mat4> get_transf()
   {
      std::vector<glm::mat4> ret;
      for (size_t i = 0; i < transf.size(); i++) {
         ret.push_back(this->model_matrix*transf[i].get_matrix());
      }
      return ret;
   }

   uint indices_to_draw(){
      return nb_indices;
   }

   virtual void draw(){

      //TODO add shadow map to trunks
      // if(has_shadow_buffer){
      //    glUniformMatrix4fv( glGetUniformLocation(_pid, "shadow_matrix"), 1, GL_FALSE, glm::value_ptr(this->shadow_matrix));
      //
      //    glUniform1i( glGetUniformLocation(_pid, "shadow_buffer_texture_width"), shadow_buffer_texture_width);
      //    glUniform1i( glGetUniformLocation(_pid, "shadow_buffer_texture_height"), shadow_buffer_texture_height);
      //
      //    glActiveTexture(GL_TEXTURE4);
      //    glBindTexture(GL_TEXTURE_2D, _shadow_texture_id);
      //    GLuint tex_id = glGetUniformLocation(_pid, "shadow_buffer_tex");
      //    glUniform1i(tex_id, 4 /*GL_TEXTURE4*/);
      // }

      glBindVertexArray(_vao);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glDrawElements(GL_TRIANGLES, nb_indices, GL_UNSIGNED_INT, 0);

      glBindVertexArray(0);
   }

protected:
   GLuint _vao;
   GLuint _vbo_pos;
   GLuint _vbo_sur_norm;
   GLuint _vbo_idx;

   unsigned int _num_vertices;

   std::vector<Transform> transf;

   GLfloat *positions;
   uint nb_positions;

   GLuint *indices;
   uint nb_indices;

   GLfloat *text_coord;
   uint nb_text_coord;

   GLfloat *normals;
   uint nb_normals;

   //generate real position from indexed ones (to do per surface normals)
   void generate_positions(GLfloat indexed_position[24], GLuint index[36], GLfloat position[36*3]){
      for (size_t i = 0; i < 36; i++) {
         position[i*3+0] = indexed_position[index[i]*3+0];
         position[i*3+1] = indexed_position[index[i]*3+1];
         position[i*3+2] = indexed_position[index[i]*3+2];
      }
   }
};

#endif
