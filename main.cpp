#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <chrono>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <IL/il.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //perspective

#include "camera.h"
#include "camera_free.h"
#include "camera_fps.h"
#include "shader_helper.h"
#include "_plane/plane.h"
#include "_cube/cube.h"
#include "_sphere/sphere.h"
#include "_terrain/terrain.h"
#include "_grass/grass_element.h"
#include "_grass/grass_manager.h"
#include "transform.h"
#include "noise_generator.hpp"
// #include "noise_generator_cached.hpp"
#include "texture.h"
#include "_quad_screen/quad_screen.h"
#include "_terrain_quad/qterrain.h"
#include "framebuffer_multisampled.hpp"
#include "framebuffer_singlesampled.hpp"
#include "drawable.h"
#include "_shadow_map/depth_framebuffer.hpp"
#include "_water/water.h"
#include "_sky/sky_sphere.h"
#include "_sky/clouds.h"
#include "_trees/trees_manager.h"

#include "_terrain_quad/qtree_test.hpp"

#define DEPTH_TEXTURE_SIZE 1024

//defines the factor for the lod
//lower = high precision terrain far away
//higher = high precision terrain near (better performance)
// takes into account the model matrix in qterrain, no need to adjust for it
#define FACTOR_DISTANCE_LOD 1.5f
//#define FACTOR_DISTANCE_LOD 2.0f

//tells, at level lod 0, which size should the sub terrain be
//if value is X, then there would be (X*2) by (X*2) squares in the terrain
//each lod level multiplies this value by 2
#define TERRAIN_INTIAL_GRANULARITY 8

//max lod levels
#define LOD_MAX_LEVELS 7

//tells that the noise generation should have less precision
//that means that a terrain of level 7 will not have a noise generation
//of level 7 but of level 7-LOD_NOISE_GEN_SIMPLIFICATION
//this allows higher precision terrain without a large terrain cost
//if too low can cause big artifacts
//noise generator max level makes more sense and replaces this
//#define LOD_NOISE_GEN_SIMPLIFICATION 0

//the level of the noise generator defines the number of passes that it will go through
//to generate the noise, there is no need to have lots of passes if the terrain is not
//granular (in terms of triangles)
//the noise generator level is chosen as a function of log2(terrain_granularity)
//to avoid having a noise generation overload at high granularity, this option allows
//to limit the level of noise gen
#define NOISE_GENERATOR_MAX_LEVEL 6

#define TERRAIN_HEIGHT 96

void init();
void display();
void cleanup();
GLuint load_shader(char *path, GLenum shader_type);

Camera *cam;

Camera cam_fixed;
Camera_free cam_free;
Camera_fps cam_fps;

Framebuffer *framebuffer;
Framebuffer *framebuffer_refraction;
Framebuffer *framebuffer_reflection;
Depth_framebuffer *depth_framebuffer;

Plane plane_test;
Grass_manager grass_manager;
Trees_manager trees_manager;
Sky_sphere sky_sphere;
Clouds clouds;
Quad_screen quad_screen;
Transform plane_test_transf;
Transform grass_element_transf;
Transform water_transf;
Transform sky_sphere_transf;
Transform clouds_transf;
Water water;
QTerrain qterrain;
Transform terrain_transf;

glm::mat4x4 projection_mat;

Noise_generator noise_gen;
// Noise_generator_cached *noise_gen;

std::vector<Drawable*> lst_drawable;

bool activate_wireframe;
GLfloat camera_position[3];
GLfloat camera_direction[3];
GLuint light_mode = 0;
bool activate_specular = true;
bool activate_spot = true;

unsigned int shadow_mapping_effect = 1;
unsigned int AO_effect = 0;
unsigned int water_effect = 1;

float sky_scale = 2048.0f;
bool advance_sky = true;

// const int win_width = 1600;
// const int win_height = 900;

const int win_width = 1280;
const int win_height = 720;

// const int win_width = 300;
// const int win_height = 200;

const float water_height = -1.0f/2.0f*TERRAIN_HEIGHT;

float time_measured;

unsigned int effect_select;

float clouds_amount = 0.5f;

int main(){
   if( !glfwInit() ){
      std::cout << "Error to initialize GLFW" << std::endl;
      return -1;
   }

   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
   glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
   glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   if( !glfwOpenWindow(win_width, win_height, 0,0,0,0, 32,0, GLFW_WINDOW) ){
      std::cout << "failed to open window" << std::endl;
      return -1;
   }

   glewExperimental = GL_TRUE;
   if(glewInit() != GLEW_NO_ERROR){
      std::cout << "glew error\n";
      return -1;
   }

   init();

   while(glfwGetKey(GLFW_KEY_ESC)!=GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED)){

      if(glfwGetKey('S') == GLFW_PRESS){
         cam->input_handling('S');
      }
      if(glfwGetKey('A') == GLFW_PRESS){
         cam->input_handling('A');
      }

      if(glfwGetKey('W') == GLFW_PRESS){
         cam->input_handling('W');
      }
      if(glfwGetKey('D') == GLFW_PRESS){
         cam->input_handling('D');
      }

      if(glfwGetKey('L') == GLFW_PRESS){
         cam->input_handling('L');
      }

      if(glfwGetKey('J') == GLFW_PRESS){
         cam->input_handling('J');
      }
      if(glfwGetKey('K') == GLFW_PRESS){
         cam->input_handling('K');
      }
      if(glfwGetKey('I') == GLFW_PRESS){
         cam->input_handling('I');
      }

      if(glfwGetKey('C') == GLFW_PRESS){ //fixed light
         activate_wireframe = false;
      }
      if(glfwGetKey('X') == GLFW_PRESS){ //moving light
         activate_wireframe = true;
      }

      if(glfwGetKey('E') == GLFW_PRESS){
         water_effect = 0;
      }
      if(glfwGetKey('R') == GLFW_PRESS){
         water_effect = 1;
      }
      if(glfwGetKey('T') == GLFW_PRESS){
         advance_sky = true;
      }
      if(glfwGetKey('Z') == GLFW_PRESS){
         advance_sky = false;
      }
      // if(glfwGetKey('U') == GLFW_PRESS){
      //    water_effect = 4;
      // }
      // if(glfwGetKey('F') == GLFW_PRESS){
      //    water_effect = 5;
      // }
      // if(glfwGetKey('G') == GLFW_PRESS){
      //    water_effect = 6;
      // }

      if(glfwGetKey('O') == GLFW_PRESS){
         if(clouds_amount > 0.0f){
            clouds_amount -= 0.002;
         }
      }

      if(glfwGetKey('P') == GLFW_PRESS){
         if(clouds_amount < 1.0f){
            clouds_amount += 0.002;
         }
      }

      static clock_t begin_time = clock();
      static int image_count = 0;

      display();
      image_count++;

      float diff_time = float( clock () - begin_time ) /  CLOCKS_PER_SEC;

      //display fps every so and so seconds
      if(diff_time > 2.0f){
         printf("fps: %f\n", image_count/diff_time);
         begin_time = clock();
         image_count = 0;
      }

      glfwSwapBuffers();
   }

   cleanup();

   return 0;
}

void init(){

   time_measured = 0.0f;
   effect_select = 0;

   //glClearColor(1.0, 1.0, 1.0, 1.0);
   glClearColor(0.3, 0.6, 1.0, 1.0); //sky

   ilInit();

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

   glEnable(GL_CULL_FACE);
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);

   glEnable(GL_MULTISAMPLE);

   glViewport(0,0,win_width,win_height);
   //projection_mat = glm::perspective(3.1415f/2.0f, (float)win_width/(float)win_height, 0.1f, 1000.0f);

   activate_wireframe = false;

   sky_sphere.init(128, 128, sky_scale);
   lst_drawable.push_back(&sky_sphere);

   plane_test.init();

   water.init();
   noise_gen.set_noise_function(NOISE_SELECT_2VORONOI_PERLIN);
   qterrain.init(&noise_gen, FACTOR_DISTANCE_LOD, TERRAIN_INTIAL_GRANULARITY, LOD_MAX_LEVELS, NOISE_GENERATOR_MAX_LEVEL);

   terrain_transf.scale(2048, TERRAIN_HEIGHT, 2048);

   qterrain.set_model_matrix(terrain_transf.get_matrix());
   lst_drawable.push_back(&qterrain);
   lst_drawable.push_back(&water);

   grass_manager.init(&qterrain, water_height+10);
   lst_drawable.push_back(&grass_manager);

   trees_manager.init(&qterrain, water_height+10);
   lst_drawable.push_back(&trees_manager);

   clouds.init();
   lst_drawable.push_back(&clouds);

   sky_sphere_transf.scale(sky_scale, sky_scale, sky_scale);

   plane_test_transf.translate(2.0f, 1.0f, 0.0f);
   plane_test_transf.rotate(0.0f, 1.0, 0.0f, 3.1415);
   plane_test_transf.rotate(1.0f, 0.0f, 0.0f, 3.1415/2.0f);

   water_transf.translate(0.0f, water_height, 0.0f);
   water_transf.scale(1024.0f, 1.0f, 1024.0f);

   framebuffer_refraction = new Framebuffer_singlesampled();
   framebuffer_refraction->init(win_width, win_height);
   framebuffer = new Framebuffer_multisampled();
   framebuffer_reflection = new Framebuffer_singlesampled();
   framebuffer_reflection->init(win_width, win_height);
   depth_framebuffer = new Depth_framebuffer();
   depth_framebuffer->init(win_width/2, win_height/2);
   GLuint tex_fb = framebuffer->init(win_width, win_height);

   water.set_texture_refraction(framebuffer_refraction->get_texture());
   water.set_texture_reflection(framebuffer_reflection->get_texture());
   water.set_texture_refraction_depth(depth_framebuffer->get_texture_id());

   quad_screen.init(tex_fb, win_width, win_height);

   cam_fixed.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fixed.set_window_size(win_width, win_height);

   cam_free.lookAt(6.0f, 6.0f, 12.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_free.set_window_size(win_width, win_height);

   cam_fps.lookAt(1.5f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
   cam_fps.init(0.5f, &qterrain);
   cam_fps.set_window_size(win_width, win_height);

   cam = &cam_free;
   cam_free.update_pos();

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_clip_coord(0, 1, 0, -water_height);
   }
}

void display(){

   cam->get_position(camera_position);
   cam->get_direction(camera_direction);

   clouds.set_level(clouds_amount);

   if(advance_sky){
      sky_sphere.advance_sun();
   }

   qterrain.update_lod_camera(camera_position);

   water.set_effect(water_effect);

   float *sun_dir = sky_sphere.get_sun_direction();
   float sun_dist = 700.0f;
   float sun_pos[3] = {-sun_dist*sun_dir[0], -sun_dist*sun_dir[1], -sun_dist*sun_dir[2]};
   depth_framebuffer->set_light_pos(sun_pos);

   //set what the depth buffer will see (for shadow map), follow the free cam
   float pos_cam[3];
   cam->get_position(pos_cam);
   //for now, y is set to 0, maybe set it to terrain height?
   depth_framebuffer->center_to(pos_cam[0], 0, pos_cam[2]);

   plane_test.set_MVP_matrices(plane_test_transf.get_matrix(), cam->getMatrix(), cam->get_perspective_mat());
   qterrain.set_MVP_matrices(terrain_transf.get_matrix(), cam->getMatrix(), cam->get_perspective_mat());

   sky_sphere.set_MVP_matrices(sky_sphere_transf.get_matrix(), cam->getMatrix(), cam->get_perspective_mat());

   water.set_MVP_matrices(water_transf.get_matrix(), cam->getMatrix(), cam->get_perspective_mat());
   clouds.set_MVP_matrices(clouds_transf.get_matrix(), cam->getMatrix(), cam->get_perspective_mat());
   water.set_time(glfwGetTime());

   for (size_t i = 0; i < lst_drawable.size(); i++) {

      lst_drawable[i]->set_camera_pos(camera_position);

      lst_drawable[i]->set_camera_direction(camera_direction);

      lst_drawable[i]->set_shadow_buffer_texture_size(win_width, win_height);
      lst_drawable[i]->set_shadow_mapping_effect(shadow_mapping_effect);
      lst_drawable[i]->set_window_dim(win_width, win_height);

      lst_drawable[i]->set_shadow_buffer_texture(depth_framebuffer->get_texture_id());
      lst_drawable[i]->set_shadow_matrix(depth_framebuffer->get_shadow_mat());

      lst_drawable[i]->set_sun_dir(sky_sphere.get_sun_direction());
      lst_drawable[i]->set_sun_col(sky_sphere.get_sun_rgb_color());
   }

   water.set_enabled(false);
   //==============REFRACTION STEP
   framebuffer_refraction->bind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   //no need for grass and tress for the refraction buffer capturing underwater scene
   trees_manager.set_enabled(false);
   grass_manager.set_enabled(false);

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->draw();
   }

   framebuffer_refraction->unbind();
   depth_framebuffer->draw_fb(&lst_drawable);
   grass_manager.set_enabled(true);
   trees_manager.set_enabled(true);

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_projection_matrix(cam->get_perspective_mat());
   }

   //==============REFLEXION STEP
   framebuffer_reflection->bind();
   glEnable(GL_CLIP_DISTANCE0);

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   float reflexion_matrix_height = water_height;

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_view_matrix(cam->get_reflection_matrix(reflexion_matrix_height));
   }

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->draw();
   }

   glDisable(GL_CLIP_DISTANCE0);
   framebuffer_reflection->unbind();

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->set_view_matrix(cam->getMatrix());
   }

   //===============FINAL STEP
   water.set_enabled(true);
   framebuffer->bind();
   if(activate_wireframe){
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   }

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);

   for (size_t i = 0; i < lst_drawable.size(); i++) {
      lst_drawable[i]->draw();
   }

   framebuffer->unbind();

   glClear(GL_COLOR_BUFFER_BIT);
   glClear(GL_DEPTH_BUFFER_BIT);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   quad_screen.draw(effect_select);
}

void cleanup(){
}
