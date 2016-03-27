#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <stack>
#include <fstream>
#include <sstream>
#include <vector>

#include <cmath>

#include "glew/glew.h"

#include "GLFW/glfw3.h"
#include "stb/stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4, glm::ivec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp" // glm::value_ptr

/* assimp include files. These three are usually needed. */
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

#if DEBUG_PRINT == 0
#define debug_print(FORMAT, ...) ((void)0)
#else
#ifdef _MSC_VER
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define debug_print(FORMAT, ...) \
    fprintf(stderr, "%s() in %s, line %i: " FORMAT "\n", \
        __func__, __FILE__, __LINE__, __VA_ARGS__)
#endif
#endif

// Font buffers
extern const unsigned char DroidSans_ttf[];
extern const unsigned int DroidSans_ttf_len;    

// Shader utils
int check_link_error(GLuint program);
int check_compile_error(GLuint shader, const char ** sourceBuffer);
int check_link_error(GLuint program);
GLuint compile_shader(GLenum shaderType, const char * sourceBuffer, int bufferSize);
GLuint compile_shader_from_file(GLenum shaderType, const char * fileName);

// OpenGL utils
bool checkError(const char* title);

struct Camera
{
    float radius;
    float theta;
    float phi;
    glm::vec3 o;
    glm::vec3 eye;
    glm::vec3 up;
};
void camera_defaults(Camera & c);
void camera_zoom(Camera & c, float factor);
void camera_turn(Camera & c, float phi, float theta);
void camera_pan(Camera & c, float x, float y);

struct GUIStates
{
    bool panLock;
    bool turnLock;
    bool zoomLock;
    int lockPositionX;
    int lockPositionY;
    int camera;
    double time;
    bool playing;
    static const float MOUSE_PAN_SPEED;
    static const float MOUSE_ZOOM_SPEED;
    static const float MOUSE_TURN_SPEED;
};
const float GUIStates::MOUSE_PAN_SPEED = 0.001f;
const float GUIStates::MOUSE_ZOOM_SPEED = 0.05f;
const float GUIStates::MOUSE_TURN_SPEED = 0.005f;
void init_gui_states(GUIStates & guiStates);

float mapRange(float old_value, float old_bottom, float old_top, float new_bottom, float new_top) {
     return (old_value - old_bottom) / (old_top - old_bottom) * (new_top - new_bottom) + new_bottom;
}

struct DataCycle
{
    glm::vec3 customColor;
    glm::vec3 customLineColor;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> linePositions;
    std::vector<float> times;
    std::vector<float> rotations;
    int currentIdTime;
    float lerpValue;
    float offsetLine;
    glm::vec3 mCurrentLinePos;

    DataCycle(std::string fpath) : offsetLine(-1.5f) {
        std::ifstream infile(fpath.c_str());
        std::string line;
        int i = 0;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            if (i == 0) {
                iss >> customColor.x >> customColor.y >> customColor.z;
            } else if (i == 1) {
                iss >> customLineColor.x >> customLineColor.y >> customLineColor.z;
            } else {
                float t, r;
                glm::vec3 pos;
                iss >> t >> pos.x >> pos.y >> pos.z >> r;
                times.push_back(t);
                rotations.push_back(r);
                positions.push_back(pos);
                float radAngle = glm::radians(r);
                glm::vec3 offset(glm::cos(radAngle), glm::sin(radAngle), 0);
                offset *= offsetLine;
                linePositions.push_back(pos + offset);
            }
            ++i;
        }
        infile.close();
        currentIdTime = 0;
        if (i < 1) {
            throw "Data cycle is bad formatted";
        }
    }

    glm::mat4 getMV(float t) {
        findCurrentLerpByTime(t);
        glm::vec3 pos = positions[currentIdTime];
        float rot = glm::radians(rotations[currentIdTime]);
        glm::vec3 cLinePos = linePositions[currentIdTime];
        if (lerpValue > 0) {
            pos = glm::mix(positions[currentIdTime], positions[currentIdTime + 1], lerpValue);
            cLinePos = glm::mix(linePositions[currentIdTime], linePositions[currentIdTime + 1], lerpValue);
            rot = glm::radians(glm::mix(rotations[currentIdTime], rotations[currentIdTime + 1], lerpValue));

        }
        mCurrentLinePos = cLinePos;
        return glm::rotate(glm::translate(glm::mat4(), pos), rot, glm::vec3(0.0, 0.0, 1.0));
    }

    void findCurrentLerpByTime(float t) {
        for (int i = currentIdTime; i < times.size() - 1; ++i) {
            if (t >= times[i] && t < times[i + 1]) {
                currentIdTime = i;
                lerpValue = mapRange(t, times[i], times[i + 1], 0.0, 1.0);
                return;
            }
        }
        if (t > *(times.end())) {
            currentIdTime = times.size() - 1;
            lerpValue = 0;
        }
    }

    void printDebug() {
        std::cout << customColor.x << " " << customColor.y << " " << customColor.z << " " << std::endl;
        for (size_t i = 0; i < times.size(); ++i) {
            std::cout << times[i] << " " << positions[i].x << " " << positions[i].y << " " << positions[i].z << std::endl;
        }
    }

};

struct DataCamera
{
    std::vector<glm::vec3> positions;
    std::vector<float> times;
    std::vector<glm::vec3> targetPositions;
    int currentIdTime;
    float lerpValue;

    DataCamera(std::string fpath) {
        std::ifstream infile(fpath.c_str());
        std::string line;
        int i = 0;
        while (std::getline(infile, line)) {
            std::istringstream iss(line);
            float t;
            glm::vec3 pos;
            glm::vec3 target;
            iss >> t >> pos.x >> pos.y >> pos.z >> target.x >> target.y >> target.z;
            times.push_back(t);
            targetPositions.push_back(target);
            positions.push_back(pos);
        }
        infile.close();
        currentIdTime = 0;
    }

    glm::mat4 getMV(float t) {
        findCurrentLerpByTime(t);
        glm::vec3 pos = positions[currentIdTime];
        glm::vec3 target = targetPositions[currentIdTime];
        if (lerpValue > 0) {
            pos = glm::mix(positions[currentIdTime], positions[currentIdTime + 1], lerpValue);
            target = glm::mix(targetPositions[currentIdTime], targetPositions[currentIdTime + 1], lerpValue);

        }
        glm::vec3 up(0.0f, 0.0f, 1.0f);
        return glm::lookAt(pos, target, up);
    }

    void findCurrentLerpByTime(float t) {
        for (int i = currentIdTime; i < times.size() - 1; ++i) {
            if (t >= times[i] && t < times[i + 1]) {
                currentIdTime = i;
                lerpValue = mapRange(t, times[i], times[i + 1], 0.0, 1.0);
                return;
            }
        }
        if (t > *(times.end())) {
            currentIdTime = times.size() - 1;
            lerpValue = 0;
        }
    }

    void printDebug() {
        for (size_t i = 0; i < times.size(); ++i) {
            std::cout << times[i] << " " << positions[i].x << " " << positions[i].y << " " << positions[i].z <<  " " << targetPositions[i].x << " " << targetPositions[i].y << " " << targetPositions[i].z << std::endl;
        }
    }

};


struct PointLight
{
   glm::vec3 position;
   glm::vec3 color;
   float intensity;
};

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};

struct SpotLight
{
    glm::vec3 position;
    float angle;
    glm::vec3 direction;
    float penumbraAngle;
    glm::vec3 color;
    float intensity;
};


std::string lightCyclePath = "./models/Light Cycle/HQ_Movie cycle.obj";

int main( int argc, char **argv )
{
    float gamma = 0.7f;
    int width = 1280, height= 720;
    float widthf = (float) width, heightf = (float) height;
    double t, deltaLoad;
    float fps = 0.f;

    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_DECORATED, GL_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    int const DPI = 2; // For retina screens only
#else
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    int const DPI = 1;
# endif

    // Open a window and create its OpenGL context
    GLFWwindow * window = glfwCreateWindow(width/DPI, height/DPI, "aogl", 0, 0);
    if( ! window )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }
    glfwMakeContextCurrent(window);

    // Init glew
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
          /* Problem: glewInit failed, something is seriously wrong. */
          fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
          exit( EXIT_FAILURE );
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );

    // Enable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );
    GLenum glerr = GL_NO_ERROR;
    glerr = glGetError();

    ImGui_ImplGlfwGL3_Init(window, true);

    // Init viewer structures
    Camera camera;
    camera_defaults(camera);
    GUIStates guiStates;
    init_gui_states(guiStates);
    int instanceCount = 1;
    float scaleFactor=1.f;

    GLuint vertShaderId = compile_shader_from_file(GL_VERTEX_SHADER, "aogl.vert");
    GLuint fragShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, "aogl.frag");
    GLuint programObject = glCreateProgram();
    glAttachShader(programObject, vertShaderId);
    glAttachShader(programObject, fragShaderId);
    glLinkProgram(programObject);
    if (check_link_error(programObject) < 0)
        exit(1);

    GLuint vertPassShaderId = compile_shader_from_file(GL_VERTEX_SHADER, "pass.vert");
    GLuint geoCubeLineShaderId = compile_shader_from_file(GL_GEOMETRY_SHADER, "cubeLine.geom");
    GLuint fragCubeLineShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, "cubeLine.frag");
    GLuint programCubeLine = glCreateProgram();
    glAttachShader(programCubeLine, vertPassShaderId);
    glAttachShader(programCubeLine, geoCubeLineShaderId);
    glAttachShader(programCubeLine, fragCubeLineShaderId);
    glLinkProgram(programCubeLine);
    if (check_link_error(programCubeLine) < 0)
        exit(1);

    GLuint vertBlitShaderId = compile_shader_from_file(GL_VERTEX_SHADER, "blit.vert");
    GLuint fragBlitShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, "blit.frag");
    GLuint blitProgramObject = glCreateProgram();
    glAttachShader(blitProgramObject, vertBlitShaderId);
    glAttachShader(blitProgramObject, fragBlitShaderId);
    glLinkProgram(blitProgramObject);
    if (check_link_error(blitProgramObject) < 0)
        exit(1);

    GLuint fragpointlightShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, "pointlight.frag");
    GLuint pointlightProgramObject = glCreateProgram();
    glAttachShader(pointlightProgramObject, vertBlitShaderId);
    glAttachShader(pointlightProgramObject, fragpointlightShaderId);
    glLinkProgram(pointlightProgramObject);
    if (check_link_error(pointlightProgramObject) < 0)
        exit(1);

   GLuint fragdirectionallightShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, "directionallight.frag");
   GLuint directionallightProgramObject = glCreateProgram();
   glAttachShader(directionallightProgramObject, vertBlitShaderId);
   glAttachShader(directionallightProgramObject, fragdirectionallightShaderId);
   glLinkProgram(directionallightProgramObject);
   if (check_link_error(directionallightProgramObject) < 0)
       exit(1);

   GLuint fragspotlightShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, "spotlight.frag");
   GLuint spotlightProgramObject = glCreateProgram();
   glAttachShader(spotlightProgramObject, vertBlitShaderId);
   glAttachShader(spotlightProgramObject, fragspotlightShaderId);
   glLinkProgram(spotlightProgramObject);
   if (check_link_error(spotlightProgramObject) < 0)
       exit(1);

   GLuint fragGammalightShaderId = compile_shader_from_file(GL_FRAGMENT_SHADER, "gamma.frag");
   GLuint gammaProgramObject = glCreateProgram();
   glAttachShader(gammaProgramObject, vertBlitShaderId);
   glAttachShader(gammaProgramObject, fragGammalightShaderId);
   glLinkProgram(gammaProgramObject);
   if (check_link_error(gammaProgramObject) < 0)
       exit(1);

    // Upload uniforms
    GLuint mvpLocation = glGetUniformLocation(programObject, "MVP");
    GLuint mvLocation = glGetUniformLocation(programObject, "MV");
    GLuint timeLocation = glGetUniformLocation(programObject, "Time");
    GLuint diffuseLocation = glGetUniformLocation(programObject, "Diffuse");
    GLuint diffuseColorLocation = glGetUniformLocation(programObject, "DiffuseColor");
    GLuint personalColorLocation = glGetUniformLocation(programObject, "PersonalColor");
    GLuint specularLocation = glGetUniformLocation(programObject, "Specular");
    GLuint specularColorLocation = glGetUniformLocation(programObject, "SpecularColor");
    GLuint lightLocation = glGetUniformLocation(programObject, "Light");
    GLuint specularPowerLocation = glGetUniformLocation(programObject, "SpecularPower");
    GLuint instanceCountLocation = glGetUniformLocation(programObject, "InstanceCount");
    GLuint diffuseColorSubLocation = glGetSubroutineUniformLocation(programObject, GL_FRAGMENT_SHADER, "DiffuseColorSub");
    GLuint specularColorSubLocation = glGetSubroutineUniformLocation(programObject, GL_FRAGMENT_SHADER, "SpecularColorSub");    
    glProgramUniform1i(programObject, diffuseLocation, 0);
    glProgramUniform1i(programObject, specularLocation, 1);

    GLuint mvpCubeLineLocation = glGetUniformLocation(programCubeLine, "MVP");
    GLuint timeCubeLineLocation = glGetUniformLocation(programCubeLine, "Time");
    GLuint personalColorCubeLineLocation = glGetUniformLocation(programCubeLine, "PersonalColor");
    GLuint TotalPointsCubeLineLocation = glGetUniformLocation(programCubeLine, "TotalPoints");
    GLuint CurrentFinalPositionCubeLineLocation = glGetUniformLocation(programCubeLine, "CurrentFinalPosition");

    GLuint pointlightColorLocation = glGetUniformLocation(pointlightProgramObject, "ColorBuffer");
    GLuint pointlightNormalLocation = glGetUniformLocation(pointlightProgramObject, "NormalBuffer");
    GLuint pointlightDepthLocation = glGetUniformLocation(pointlightProgramObject, "DepthBuffer");
    GLuint pointlightPositionLightLocation = glGetUniformLocation(pointlightProgramObject, "LightPosition");
    GLuint pointlightColorLightLocation = glGetUniformLocation(pointlightProgramObject, "LightColor");
    GLuint pointlightIntensityLightLocation = glGetUniformLocation(pointlightProgramObject, "LightIntensity");
    GLuint pointInverseProjectionLocation = glGetUniformLocation(pointlightProgramObject, "InverseProjection");
    glProgramUniform1i(pointlightProgramObject, pointlightColorLocation, 0);
    glProgramUniform1i(pointlightProgramObject, pointlightNormalLocation, 1);
    glProgramUniform1i(pointlightProgramObject, pointlightDepthLocation, 2);
    glm::mat4 projection = glm::perspective(45.0f, widthf / heightf, 0.01f, 600.f);
    glm::mat4 inverseProjection = glm::inverse(projection);
    glProgramUniformMatrix4fv(pointlightProgramObject, pointInverseProjectionLocation, 1, 0, glm::value_ptr(inverseProjection));

    GLuint directionallightColorLocation = glGetUniformLocation(directionallightProgramObject, "ColorBuffer");
    GLuint directionallightNormalLocation = glGetUniformLocation(directionallightProgramObject, "NormalBuffer");
    GLuint directionallightDepthLocation = glGetUniformLocation(directionallightProgramObject, "DepthBuffer");
    GLuint directionallightDirectionLightLocation = glGetUniformLocation(directionallightProgramObject, "LightDirection");
    GLuint directionallightColorLightLocation = glGetUniformLocation(directionallightProgramObject, "LightColor");
    GLuint directionallighttIntensityLightLocation = glGetUniformLocation(directionallightProgramObject, "LightIntensity");
    GLuint directionallightInverseProjectionLocation = glGetUniformLocation(directionallightProgramObject, "InverseProjection");
    glProgramUniform1i(directionallightProgramObject, directionallightColorLocation, 0);
    glProgramUniform1i(directionallightProgramObject, directionallightNormalLocation, 1);
    glProgramUniform1i(directionallightProgramObject, directionallightDepthLocation, 2);
    glProgramUniformMatrix4fv(directionallightProgramObject, directionallightInverseProjectionLocation, 1, 0, glm::value_ptr(inverseProjection));

    GLuint spotlightColorLocation = glGetUniformLocation(spotlightProgramObject, "ColorBuffer");
   GLuint spotlightNormalLocation = glGetUniformLocation(spotlightProgramObject, "NormalBuffer");
   GLuint spotlightDepthLocation = glGetUniformLocation(spotlightProgramObject, "DepthBuffer");
   GLuint spotlightPositionLightLocation = glGetUniformLocation(spotlightProgramObject, "LightPosition");
   GLuint spotlightDirectionLightLocation = glGetUniformLocation(spotlightProgramObject, "LightDirection");
   GLuint spotlightColorLightLocation = glGetUniformLocation(spotlightProgramObject, "LightColor");
   GLuint spotlightAngleLightLocation = glGetUniformLocation(spotlightProgramObject, "LightAngle");
   GLuint spotlightPenumbraAngleLightLocation = glGetUniformLocation(spotlightProgramObject, "LightPenumbraAngle");
   GLuint spotlightIntensityLightLocation = glGetUniformLocation(spotlightProgramObject, "LightIntensity");
   GLuint spotInverseProjectionLocation = glGetUniformLocation(spotlightProgramObject, "InverseProjection");
   glProgramUniform1i(spotlightProgramObject, spotlightColorLocation, 0);
   glProgramUniform1i(spotlightProgramObject, spotlightNormalLocation, 1);
   glProgramUniform1i(spotlightProgramObject, spotlightDepthLocation, 2);
   glProgramUniformMatrix4fv(spotlightProgramObject, spotInverseProjectionLocation, 1, 0, glm::value_ptr(inverseProjection));

   GLuint gammaTextureLocation = glGetUniformLocation(gammaProgramObject, "Texture");
   glProgramUniform1i(gammaProgramObject, gammaTextureLocation, 0);
   GLuint gammaGammaLocation = glGetUniformLocation(gammaProgramObject, "Gamma");
   GLuint widthGammaLocation = glGetUniformLocation(gammaProgramObject, "Width");
   glProgramUniform1f(gammaProgramObject, widthGammaLocation, width);
   GLuint heightGammaLocation = glGetUniformLocation(gammaProgramObject, "Height");
   glProgramUniform1f(gammaProgramObject, heightGammaLocation, height);
   GLuint timeGammaLocation = glGetUniformLocation(gammaProgramObject, "Time");

    if (!checkError("Uniforms"))
        exit(1);

    // Subroutines index
    GLuint diffuseUniformIndex = glGetSubroutineIndex( programObject, GL_FRAGMENT_SHADER,"diffuseUniform");
    GLuint diffuseTextureIndex = glGetSubroutineIndex( programObject, GL_FRAGMENT_SHADER,"diffuseTexture");
    GLuint specularUniformIndex = glGetSubroutineIndex( programObject, GL_FRAGMENT_SHADER,"specularUniform");
    GLuint specularTextureIndex = glGetSubroutineIndex( programObject, GL_FRAGMENT_SHADER,"specularTexture");
    if (!checkError("Subroutines index"))
        exit(1);

    const aiScene* scene = NULL;
    scene = aiImportFile(lightCyclePath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

    if (!scene) {
          fprintf(stderr, "Error: impossible to open %s\n", lightCyclePath.c_str());
          exit( EXIT_FAILURE );
    }

    GLuint * assimp_vao = new GLuint[scene->mNumMeshes];
    glm::mat4 * assimp_objectToWorld = new glm::mat4[scene->mNumMeshes];
    glGenVertexArrays(scene->mNumMeshes, assimp_vao);
    GLuint * assimp_vbo = new GLuint[scene->mNumMeshes*4];
    glGenBuffers(scene->mNumMeshes*4, assimp_vbo);

    float * assimp_diffuse_colors = new float[scene->mNumMeshes*3];
    GLuint * assimp_diffuse_texture_ids = new GLuint[scene->mNumMeshes*3];
    float * assimp_specular_colors = new float[scene->mNumMeshes*3];
    GLuint * assimp_specular_texture_ids = new GLuint[scene->mNumMeshes*3];

    for (unsigned int i =0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* m = scene->mMeshes[i];
        GLuint * faces = new GLuint[m->mNumFaces*3];
        for (unsigned int j = 0; j < m->mNumFaces; ++j)
        {
            const aiFace& f = m->mFaces[j];
            faces[j*3] = f.mIndices[0];
            faces[j*3+1] = f.mIndices[1];
            faces[j*3+2] = f.mIndices[2];
        }
      
        glBindVertexArray(assimp_vao[i]);
        // Bind indices and upload data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, assimp_vbo[i*4]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->mNumFaces*3 *sizeof(GLuint), faces, GL_STATIC_DRAW);

        // Bind vertices and upload data
        glBindBuffer(GL_ARRAY_BUFFER, assimp_vbo[i*4+1]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, m->mNumVertices*3*sizeof(float), m->mVertices, GL_STATIC_DRAW);
        
        // Bind normals and upload data
        glBindBuffer(GL_ARRAY_BUFFER, assimp_vbo[i*4+2]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, m->mNumVertices*3*sizeof(float), m->mNormals, GL_STATIC_DRAW);
        
        // Bind uv coords and upload data
        glBindBuffer(GL_ARRAY_BUFFER, assimp_vbo[i*4+3]);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
        if (m->HasTextureCoords(0))
            glBufferData(GL_ARRAY_BUFFER, m->mNumVertices*3*sizeof(float), m->mTextureCoords[0], GL_STATIC_DRAW);
        else
            glBufferData(GL_ARRAY_BUFFER, m->mNumVertices*3*sizeof(float), m->mVertices, GL_STATIC_DRAW);
        delete[] faces;

        const aiMaterial * mat = scene->mMaterials[m->mMaterialIndex];
        int texIndex = 0;
        std::string path = lightCyclePath;
        size_t pos = path.find_last_of("\\/");
        std::string basePath = (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
        aiString texPath;

        // Diffuse
        if(AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath))
        {
            std::string fileloc = basePath + texPath.data;
            pos = fileloc.find_last_of("\\");
            fileloc = (std::string::npos == pos) ? fileloc : fileloc.replace(pos, 1, "/");
            int x;
            int y;
            int comp;
            glGenTextures(1, assimp_diffuse_texture_ids + i);
            unsigned char * diffuse = stbi_load(fileloc.c_str(), &x, &y, &comp, 3);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, assimp_diffuse_texture_ids[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuse);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            assimp_diffuse_texture_ids[i] = 0;
        }

        aiColor4D diffuse;
        if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        {
            assimp_diffuse_colors[i*3] = diffuse.r;
            assimp_diffuse_colors[i*3+1] = diffuse.g;
            assimp_diffuse_colors[i*3+2] = diffuse.b;
        }
        else
        {
            assimp_diffuse_colors[i*3] = 1.f;
            assimp_diffuse_colors[i*3+1] = 0.f;
            assimp_diffuse_colors[i*3+2] = 1.f;
        }

        // SPECULAR
        if(AI_SUCCESS == mat->GetTexture(aiTextureType_SPECULAR, texIndex, &texPath))
        {
            std::string fileloc = basePath + texPath.data;
            pos = fileloc.find_last_of("\\");
            fileloc = (std::string::npos == pos) ? fileloc : fileloc.replace(pos, 1, "/");
            int x;
            int y;
            int comp;
            glGenTextures(1, assimp_specular_texture_ids + i);
            unsigned char * specular = stbi_load(fileloc.c_str(), &x, &y, &comp, 3);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, assimp_specular_texture_ids[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, specular);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            assimp_specular_texture_ids[i] = 0;
        }

        aiColor4D specular;
        if(AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &specular))
        {
            assimp_specular_colors[i*3] = specular.r;
            assimp_specular_colors[i*3+1] = specular.g;
            assimp_specular_colors[i*3+2] = specular.b;
        }
        else
        {
            assimp_specular_colors[i*3] = 1.f;
            assimp_specular_colors[i*3+1] = 1.f;
            assimp_specular_colors[i*3+2] = 1.f;
        }
    }

    std::stack<aiNode*> stack;
    stack.push(scene->mRootNode);
    while(stack.size()>0)
    {
        aiNode * node = stack.top();
        for (unsigned int i =0; i < node->mNumMeshes; ++i)
        {
            unsigned int mId = node->mMeshes[i];
            aiMatrix4x4 t = node->mTransformation;
            assimp_objectToWorld[i] = glm::mat4(t.a1, t.a2, t.a3, t.a4,
                                                t.b1, t.b2, t.b3, t.b4,
                                                t.c1, t.c2, t.c3, t.c4,
                                                t.d1, t.d2, t.d3, t.d4);
        }
        stack.pop();
    }

    // Unbind everything. Potentially illegal on some implementations
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // Cycles data
    DataCycle* dataCycles[] { new DataCycle("./dataCycle1.txt"), new DataCycle("./dataCycle2.txt")};
    DataCamera dataCamera("./dataCamera.txt");

    GLuint vaoLineCycles[2];
    GLuint vboLineCycles[2];
    glGenBuffers(2, vboLineCycles);
    glGenVertexArrays(2, vaoLineCycles);
    float datas[] = { 0, 1, 0, 0, 2, 0, 1, 1, 1, 2, 2, 0, 0, 1, 2, 3, 3, 0, -3, 1, 0 };
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vboLineCycles[i]);
        DataCycle& dataCycle = *dataCycles[i];
        glBufferData(GL_ARRAY_BUFFER, dataCycle.linePositions.size() * 3 * sizeof(float), &dataCycle.linePositions[0], GL_STATIC_DRAW);
        glBindVertexArray(vaoLineCycles[i]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    }

    // Ground
    int plane_triangleCount = 2;
    float groundSize = 1000.0;
    int plane_triangleList[] = {0, 1, 2, 2, 1, 3};
    float plane_uvs[] = {0.f, 0.f, 0.f, groundSize, groundSize, 0.f, groundSize, groundSize};
    float plane_vertices[] = {-groundSize, 0.0, groundSize, groundSize, 0.0, groundSize, -groundSize, 0.0, -groundSize, groundSize, 0.0, -groundSize};
    float plane_normals[] = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};
    GLuint vboGround[4];
    GLuint vaoGround;
    glGenVertexArrays(1, &vaoGround);
    glGenBuffers(4 , vboGround);
    glBindVertexArray(vaoGround);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboGround[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_triangleList), plane_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vboGround[1]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);
    // Bind normals and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vboGround[2]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_normals), plane_normals, GL_STATIC_DRAW);
    // Bind uv coords and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vboGround[3]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_uvs), plane_uvs, GL_STATIC_DRAW);
    GLuint textureGround;
    glGenTextures(1, &textureGround);
    int x;
    int y;
    int comp;
    unsigned char * diffuse = stbi_load("the_grid.png", &x, &y, &comp, 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureGround);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuse);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    fprintf(stderr, "Diffuse %dx%d:%d\n", x, y, comp);

    // GBUFFER for deferred lighthing
    // Framebuffer object handle
    GLuint gbufferFbo;
    // Texture handles
    GLuint gbufferTextures[3];
    glGenTextures(3, gbufferTextures);
    // 2 draw buffers for color and normal
    GLuint gbufferDrawBuffers[2];
    // Create color texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Create normal texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Create depth texture
    glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Create Framebuffer Object
    glGenFramebuffers(1, &gbufferFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
    // Initialize DrawBuffers
    gbufferDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    gbufferDrawBuffers[1] = GL_COLOR_ATTACHMENT1;
    glDrawBuffers(2, gbufferDrawBuffers);
    // Attach textures to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, gbufferTextures[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D, gbufferTextures[1], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbufferTextures[2], 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Error on building framebuffer\n");
        exit( EXIT_FAILURE );
    }
    int   quad_triangleCount = 2;
    int   quad_triangleList[] = {0, 1, 2, 2, 1, 3};
    float quad_vertices[] =  {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
    // Quad
    GLuint vaoQuad;
    GLuint vboQuad[2];
    glGenVertexArrays(1, &vaoQuad);
    glGenBuffers(2 , vboQuad);
    glBindVertexArray(vaoQuad);
    // Bind indices and upload data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboQuad[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_triangleList), quad_triangleList, GL_STATIC_DRAW);
    // Bind vertices and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad[1]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);


    // Create Fx Framebuffer Object
    GLuint fxFbo;
    GLuint fxDrawBuffers[1];
    glGenFramebuffers(1, &fxFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fxFbo);
    fxDrawBuffers[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, fxDrawBuffers);

    // Create Fx textures
    const int FX_TEXTURE_COUNT = 4;
    GLuint fxTextures[FX_TEXTURE_COUNT];
    glGenTextures(FX_TEXTURE_COUNT, fxTextures);
    for (int i = 0; i < FX_TEXTURE_COUNT; ++i)
    {
       glBindTexture(GL_TEXTURE_2D, fxTextures[i]);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
       glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
       glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Attach first fx texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, fxTextures[0], 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
       fprintf(stderr, "Error on building framebuffer\n");
       exit( EXIT_FAILURE );
    }

    // Back to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Viewport 
    glViewport( 0, 0, width, height  );

    deltaLoad = glfwGetTime();

    do
    {
        t = glfwGetTime() - deltaLoad;
        t /= 1.0;
        ImGui_ImplGlfwGL3_NewFrame();
        //std::cout << camera.eye.x << " " << camera.eye.y << " " << camera.eye.z << " " << camera.o.x << " " << camera.o.y << " " << camera.o.z <<  std::endl;

        // Mouse states
        int leftButton = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT );
        int rightButton = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT );
        int middleButton = glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_MIDDLE );

        if( leftButton == GLFW_PRESS )
            guiStates.turnLock = true;
        else
            guiStates.turnLock = false;

        if( rightButton == GLFW_PRESS )
            guiStates.zoomLock = true;
        else
            guiStates.zoomLock = false;

        if( middleButton == GLFW_PRESS )
            guiStates.panLock = true;
        else
            guiStates.panLock = false;

        // Camera movements
        int altPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
        if (!altPressed && (leftButton == GLFW_PRESS || rightButton == GLFW_PRESS || middleButton == GLFW_PRESS))
        {
            double x; double y;
            glfwGetCursorPos(window, &x, &y);
            guiStates.lockPositionX = x;
            guiStates.lockPositionY = y;
        }
        if (altPressed == GLFW_PRESS)
        {
            double mousex; double mousey;
            glfwGetCursorPos(window, &mousex, &mousey);
            int diffLockPositionX = mousex - guiStates.lockPositionX;
            int diffLockPositionY = mousey - guiStates.lockPositionY;
            if (guiStates.zoomLock)
            {
                float zoomDir = 0.0;
                if (diffLockPositionX > 0)
                    zoomDir = -1.f;
                else if (diffLockPositionX < 0 )
                    zoomDir = 1.f;
                camera_zoom(camera, zoomDir * GUIStates::MOUSE_ZOOM_SPEED);
            }
            else if (guiStates.turnLock)
            {
                camera_turn(camera, diffLockPositionY * GUIStates::MOUSE_TURN_SPEED,
                            diffLockPositionX * GUIStates::MOUSE_TURN_SPEED);

            }
            else if (guiStates.panLock)
            {
                camera_pan(camera, diffLockPositionX * GUIStates::MOUSE_PAN_SPEED,
                            diffLockPositionY * GUIStates::MOUSE_PAN_SPEED);
            }
            guiStates.lockPositionX = mousex;
            guiStates.lockPositionY = mousey;
        }


        // Bind gbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
        glClearColor(31/255.0, 43/255.0, 43/255.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Default states
        glEnable(GL_DEPTH_TEST);


        // Get camera matrices
        glm::mat4 worldToView = glm::lookAt(camera.eye, camera.o, camera.up);
        //glm::mat4 worldToView = dataCamera.getMV(t);
        glm::mat4 objectToWorld;
        glm::mat4 mv = worldToView * objectToWorld;
        glm::mat4 mvp = projection * mv;
        glm::vec4 light = worldToView * glm::vec4(10.0, 10.0, 0.0, 1.0);

        // Select shader
        glUseProgram(programObject);

        // Upload uniforms
        glProgramUniformMatrix4fv(programObject, mvpLocation, 1, 0, glm::value_ptr(mvp));
        glProgramUniformMatrix4fv(programObject, mvLocation, 1, 0, glm::value_ptr(mv));
        glProgramUniform3fv(programObject, lightLocation, 1, glm::value_ptr(glm::vec3(light) / light.w));
        glProgramUniform1i(programObject, instanceCountLocation, (int) instanceCount);
        glProgramUniform1f(programObject, specularPowerLocation, 30.f);
        glProgramUniform1f(programObject, timeLocation, t);

        // Render vaos
        // Draw cycles
        glm::mat4 mvCycles[] { dataCycles[0]->getMV(t), dataCycles[1]->getMV(t)};

        glActiveTexture(GL_TEXTURE0);
        GLuint subIndexes[2];
        for (unsigned int i =0; i < scene->mNumMeshes; ++i)
        {
            subIndexes[0] = diffuseUniformIndex;
            if (assimp_diffuse_texture_ids[i] > 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, assimp_diffuse_texture_ids[i]);
                subIndexes[0] = diffuseTextureIndex;
            }
            glProgramUniform3fv(programObject, diffuseColorLocation, 1, assimp_diffuse_colors + 3*i);

            subIndexes[1] = specularUniformIndex;
            if (assimp_specular_texture_ids[i] > 0) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, assimp_specular_texture_ids[i]);
                subIndexes[1] = specularTextureIndex;
            }

            glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subIndexes);

            glProgramUniform3fv(programObject, specularColorLocation, 1, assimp_specular_colors + 3*i);
            const aiMesh* m = scene->mMeshes[i];
            glBindVertexArray(assimp_vao[i]);

            // Draw motos
            for (int i = 0; i < 2; ++i) {
                glProgramUniform3fv(programObject, personalColorLocation, 1, glm::value_ptr(dataCycles[i]->customColor));
                mv = worldToView * mvCycles[i] * assimp_objectToWorld[i];
                mvp = projection * mv;
                glProgramUniformMatrix4fv(programObject, mvpLocation, 1, 0, glm::value_ptr(mvp));
                glProgramUniformMatrix4fv(programObject, mvLocation, 1, 0, glm::value_ptr(mv));
                glDrawElements(GL_TRIANGLES, m->mNumFaces * 3, GL_UNSIGNED_INT, (void*)0);

            }
        }

        // Draw ground
        mv = worldToView * glm::rotate(glm::translate(glm::mat4(), glm::vec3(groundSize / 2, groundSize / 2, 0.0)), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        mvp = projection * mv;
        glProgramUniformMatrix4fv(programObject, mvpLocation, 1, 0, glm::value_ptr(mvp));
        glProgramUniformMatrix4fv(programObject, mvLocation, 1, 0, glm::value_ptr(mv));
        glActiveTexture(GL_TEXTURE0);
        subIndexes[0] = diffuseTextureIndex;
        subIndexes[1] = specularUniformIndex;
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 2, subIndexes);
        glProgramUniform3fv(programObject, specularColorLocation, 1, glm::value_ptr(glm::vec3(0.2f)));
        glBindTexture(GL_TEXTURE_2D, textureGround);
        glBindVertexArray(vaoGround);
        glDrawElements(GL_TRIANGLES, plane_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);


        // Bind gbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fxFbo);
        // Attach first fx texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, fxTextures[0], 0);
        glClearColor(31/255.0, 43/255.0, 43/255.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        // Select textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbufferTextures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gbufferTextures[1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gbufferTextures[2]);

        // Bind the same VAO for all lights
        glBindVertexArray(vaoQuad);

        // Render point lights
        glUseProgram(pointlightProgramObject);

        int pointLightCount = 50;
        float timeExplose = 25.0f;
        if (t > timeExplose) {
            pointLightCount++;
        }
        int byWidth = 10;
        int byHeight = 10;
        int space = 50;
        for (int i = 0; i < pointLightCount; ++i)
        {
           // Cycle point lights
           PointLight p;
           if (i < 2) {
               p = {
                   glm::vec3( worldToView * glm::vec4(dataCycles[i]->mCurrentLinePos + glm::vec3(0.0, 0.0, 0.83), 1.0)),
                   dataCycles[i]->customColor,
                   1.7f
               };
           } else if ( t > timeExplose && i == pointLightCount - 1) {
               p = {
                   glm::vec3( worldToView * glm::vec4(0, 0, 3.8, 1.0)),
                   glm::vec3(1.0, 0.7, 0.7),
                   exp((t - timeExplose) * 10)
               };
           } else {
               int j = i - 2;
               p = {
                   glm::vec3( worldToView * glm::vec4((j * space) % (space * byWidth) + cos(t + i) * 10, (j / byWidth) * space, 20.0 - cos(t + i) * 5  + sin(t - i) * 10, 1.0)),
                   glm::vec3(0, fabs(sin(t + i)), 1.0),
                   30.7f + sin(t + i) * 3
               };
           }
           glProgramUniform3fv(pointlightProgramObject, pointlightColorLightLocation, 1, glm::value_ptr(p.color));
           glProgramUniform3fv(pointlightProgramObject, pointlightPositionLightLocation, 1, glm::value_ptr(p.position));
           glProgramUniform1f(pointlightProgramObject, pointlightIntensityLightLocation, p.intensity);
           glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }

        glUseProgram(directionallightProgramObject);
        for (int i = 0; i < 1; ++i)
        {
             DirectionalLight d = {
                 glm::vec3( worldToView * glm::vec4(0.2, 0.1, -1.0, 1.0)),
                 glm::vec3(0.5, 1.0, 1.0),
                 0.4f
            };
             glProgramUniform3fv(directionallightProgramObject, directionallightColorLightLocation, 1, glm::value_ptr(d.color));
             glProgramUniform3fv(directionallightProgramObject, directionallightDirectionLightLocation, 1, glm::value_ptr(d.direction));
             glProgramUniform1f(directionallightProgramObject, directionallighttIntensityLightLocation, d.intensity);
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }

        glUseProgram(spotlightProgramObject);
        int spotLightCount = 2;
        for (int i = 0; i < spotLightCount; ++i)
        {
            glm::vec3 posCycle =  dataCycles[i]->mCurrentLinePos;
            SpotLight s = {
                glm::vec3(worldToView *  glm::vec4(posCycle + glm::vec3(cos(t * 3) * 3, cos(t *2) * 4, 10.0), 1.0)), 80.0f,
                glm::vec3(worldToView *  glm::vec4(glm::vec3(cos(t + 1.2) * 0.05, cos(t) * 0.1, -1.0), 0.0)), 90.0f,
                glm::vec3(1.0, 1.0, 1.0), 20.0
            };
            glProgramUniform3fv(spotlightProgramObject, spotlightColorLightLocation, 1, glm::value_ptr(s.color));
            glProgramUniform3fv(spotlightProgramObject, spotlightDirectionLightLocation, 1, glm::value_ptr(s.direction));
            glProgramUniform3fv(spotlightProgramObject, spotlightPositionLightLocation, 1, glm::value_ptr(s.position));
            glProgramUniform1f(spotlightProgramObject, spotlightIntensityLightLocation, s.intensity);
            glProgramUniform1f(spotlightProgramObject, spotlightAngleLightLocation, s.angle);
            glProgramUniform1f(spotlightProgramObject, spotlightPenumbraAngleLightLocation, s.penumbraAngle);
            glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);
        }
        glDisable(GL_BLEND);


        // Bind gbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(vaoQuad);
        // Attach first fx texture to framebuffer
        glClearColor(31/255.0, 43/255.0, 43/255.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(gammaProgramObject);
        glProgramUniform1f(gammaProgramObject, gammaGammaLocation, gamma);
        glProgramUniform1f(gammaProgramObject, timeGammaLocation, t);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fxTextures[0]);
        glDrawElements(GL_TRIANGLES, quad_triangleCount * 3, GL_UNSIGNED_INT, (void*)0);

        // Draw line cycles above lights
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND) ;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
        glDisable(GL_CULL_FACE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, gbufferFbo);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glUseProgram(programCubeLine);
        // Upload uniforms
        mv = worldToView;
        mvp = projection * worldToView;
        glProgramUniformMatrix4fv(programCubeLine, mvpCubeLineLocation, 1, 0, glm::value_ptr(mvp));
        glProgramUniform1f(programCubeLine, timeCubeLineLocation, t);
        for (int i = 0; i < 2; ++i) {
            glProgramUniform3fv(programCubeLine, personalColorCubeLineLocation, 1, glm::value_ptr(dataCycles[i]->customLineColor));
            glBindVertexArray(vaoLineCycles[i]);
            int totalPoints = dataCycles[i]->currentIdTime + 2;
            glProgramUniform1i(programCubeLine, TotalPointsCubeLineLocation, totalPoints);
            glProgramUniform4fv(programCubeLine, CurrentFinalPositionCubeLineLocation, 1, glm::value_ptr(glm::vec4(dataCycles[i]->mCurrentLinePos, 1)));
            glDrawArrays(GL_LINE_STRIP, 0, totalPoints);
        }
        glDisable(GL_BLEND) ;
        glEnable(GL_CULL_FACE);

        // Check for errors
        checkError("End loop");

        glfwSwapBuffers(window);
        glfwPollEvents();
    } // Check if the ESC key was pressed
    while( glfwGetKey( window, GLFW_KEY_ESCAPE ) != GLFW_PRESS );

    // Close OpenGL window and terminate GLFW
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    exit( EXIT_SUCCESS );
}

// No windows implementation of strsep
char * strsep_custom(char **stringp, const char *delim)
{
    register char *s;
    register const char *spanp;
    register int c, sc;
    char *tok;
    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s; ; ) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    return 0;
}

int check_compile_error(GLuint shader, const char ** sourceBuffer)
{
    // Get error log size and print it eventually
    int logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char * log = new char[logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        char *token, *string;
        string = strdup(sourceBuffer[0]);
        int lc = 0;
        while ((token = strsep_custom(&string, "\n")) != NULL) {
           printf("%3d : %s\n", lc, token);
           ++lc;
        }
        fprintf(stderr, "Compile : %s", log);
        delete[] log;
    }
    // If an error happend quit
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
        return -1;     
    return 0;
}

int check_link_error(GLuint program)
{
    // Get link error log size and print it eventually
    int logLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char * log = new char[logLength];
        glGetProgramInfoLog(program, logLength, &logLength, log);
        fprintf(stderr, "Link : %s \n", log);
        delete[] log;
    }
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);        
    if (status == GL_FALSE)
        return -1;
    return 0;
}


GLuint compile_shader(GLenum shaderType, const char * sourceBuffer, int bufferSize)
{
    GLuint shaderObject = glCreateShader(shaderType);
    const char * sc[1] = { sourceBuffer };
    glShaderSource(shaderObject, 
                   1, 
                   sc,
                   NULL);
    glCompileShader(shaderObject);
    check_compile_error(shaderObject, sc);
    return shaderObject;
}

GLuint compile_shader_from_file(GLenum shaderType, const char * path)
{
    FILE * shaderFileDesc = fopen( path, "rb" );
    if (!shaderFileDesc)
        return 0;
    fseek ( shaderFileDesc , 0 , SEEK_END );
    long fileSize = ftell ( shaderFileDesc );
    rewind ( shaderFileDesc );
    char * buffer = new char[fileSize + 1];
    fread( buffer, 1, fileSize, shaderFileDesc );
    buffer[fileSize] = '\0';
    GLuint shaderObject = compile_shader(shaderType, buffer, fileSize );
    delete[] buffer;
    return shaderObject;
}


bool checkError(const char* title)
{
    int error;
    if((error = glGetError()) != GL_NO_ERROR)
    {
        std::string errorString;
        switch(error)
        {
        case GL_INVALID_ENUM:
            errorString = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorString = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errorString = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            errorString = "GL_OUT_OF_MEMORY";
            break;
        default:
            errorString = "UNKNOWN";
            break;
        }
        fprintf(stdout, "OpenGL Error(%s): %s\n", errorString.c_str(), title);
    }
    return error == GL_NO_ERROR;
}

void camera_compute(Camera & c)
{
    c.eye.x = cos(c.theta) * sin(c.phi) * c.radius + c.o.x;   
    c.eye.y = cos(c.phi) * c.radius + c.o.y ;
    c.eye.z = sin(c.theta) * sin(c.phi) * c.radius + c.o.z;   
    c.up = glm::vec3(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
}

void camera_defaults(Camera & c)
{
    c.phi = 3.14/2.f;
    c.theta = 3.14/2.f;
    c.radius = 10.f;
    camera_compute(c);
}

void camera_zoom(Camera & c, float factor)
{
    c.radius += factor * c.radius ;
    if (c.radius < 0.1)
    {
        c.radius = 10.f;
        c.o = c.eye + glm::normalize(c.o - c.eye) * c.radius;
    }
    camera_compute(c);
}

void camera_turn(Camera & c, float phi, float theta)
{
    c.theta += 1.f * theta;
    c.phi   -= 1.f * phi;
    if (c.phi >= (2 * M_PI) - 0.1 )
        c.phi = 0.00001;
    else if (c.phi <= 0 )
        c.phi = 2 * M_PI - 0.1;
    camera_compute(c);
}

void camera_pan(Camera & c, float x, float y)
{
    glm::vec3 up(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
    glm::vec3 fwd = glm::normalize(c.o - c.eye);
    glm::vec3 side = glm::normalize(glm::cross(fwd, up));
    c.up = glm::normalize(glm::cross(side, fwd));
    c.o[0] += up[0] * y * c.radius * 2;
    c.o[1] += up[1] * y * c.radius * 2;
    c.o[2] += up[2] * y * c.radius * 2;
    c.o[0] -= side[0] * x * c.radius * 2;
    c.o[1] -= side[1] * x * c.radius * 2;
    c.o[2] -= side[2] * x * c.radius * 2;       
    camera_compute(c);
}

void init_gui_states(GUIStates & guiStates)
{
    guiStates.panLock = false;
    guiStates.turnLock = false;
    guiStates.zoomLock = false;
    guiStates.lockPositionX = 0;
    guiStates.lockPositionY = 0;
    guiStates.camera = 0;
    guiStates.time = 0.0;
    guiStates.playing = false;
}
