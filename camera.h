#ifndef CAMERA_H
#define CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera{
public:
   Camera(){

   }

   virtual void lookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up){
      this->eye = eye;
      this->center = center;
      this->up = up;
   }

   virtual void lookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ){
      lookAt(glm::vec3(eyeX, eyeY, eyeZ), glm::vec3(centerX, centerY, centerZ), glm::vec3(upX, upY, upZ));
   }

   void get_position(float position[3]){
      position[0] = eye.x;
      position[1] = eye.y;
      position[2] = eye.z;
   }

   void get_direction(float direction[3]){
      direction[0] = center.x-eye.x;
      direction[1] = center.y-eye.y;
      direction[2] = center.z-eye.z;
   }


   glm::mat4x4 getMatrix(){
      return glm::lookAt(eye, center, up);
   }

   glm::mat4x4 get_reflection_matrix(float height){

      return glm::lookAt(glm::vec3(eye.x, -eye.y+height*2, eye.z), glm::vec3(center.x, -center.y+height*2, center.z), up);
   }

   glm::mat4x4 get_perspective_mat(){
      return glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 4096.0f);
   }

   virtual void input_handling(char){

   }

   virtual void update_pos(){

   }

   virtual void set_window_size(unsigned int win_width, unsigned int win_height){
      this->win_width = win_width;
      this->win_height = win_height;
   }

protected:
   //for the lookat functon
   glm::vec3 eye;
   glm::vec3 center;
   glm::vec3 up;

   //not really part of the camera or is it? (its for the perspective matrix)
   unsigned int win_width;
   unsigned int win_height;
};

#endif
