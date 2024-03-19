#ifndef MODEL_H
#define MODEL_H

#include "material.h"
#include "texture.h"
#include "triangle.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using std::vector;
using std::string;

class model {
public:
  triangle* primitives;
  int primitives_count = 0;
  
  std::map<std::string, shared_ptr<material>> materials_loaded;
  std::map<std::string, shared_ptr<texture>> textures_loaded;  // stores all the textures loaded so far,
                                                                  // optimization to make sure textures aren't loaded more than once.


  // constructor, expects a filepath to a 3D model.
  model(const char* path) {
    
    default_diffuse = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));
    default_emissive = make_shared<solid_color>(0, 0, 0);
    default_mat = make_shared<lambertian>(default_diffuse);
    load_model(path);
  }

  ~model() {
    delete [] primitives;
  }

private:
  string directory;

  shared_ptr<texture> default_diffuse;
  shared_ptr<texture> default_emissive;
  shared_ptr<material> default_mat;

  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void load_model(string const &path) {
    Assimp::Importer import;
    import.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    const aiScene* scene = import.ReadFile(path,
                     aiProcess_Triangulate            |
                     aiProcess_JoinIdenticalVertices  |
                     aiProcess_SortByPType);
                     // | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace

    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      std::clog << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
      return;
    }
 
    directory = path.substr(0, path.find_last_of('/'));
    
    // allocate the faces array
    int primitives_count = calculate_number_of_faces(scene->mRootNode, scene);
    primitives = new triangle[primitives_count];
    
    process_node(scene->mRootNode, scene);
  }
  
  int calculate_number_of_faces (const aiNode *node, const aiScene* scene) {
    int sum = 0;
    
    // process all node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      sum += calculate_number_of_faces(mesh);
    }
    
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      sum += calculate_number_of_faces(node->mChildren[i], scene);
    }
    
    return sum;
  }
  
  int calculate_number_of_faces (const aiMesh* _mesh) {
    return _mesh->mNumFaces;
  }
 
  void process_node(aiNode *node, const aiScene *scene) {
    // process all node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      process_mesh(mesh, scene);
    }
    
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      process_node(node->mChildren[i], scene);
    }
  }
 
  void process_mesh(aiMesh* _mesh, const aiScene* scene) {
    std::clog << "Mesh has: " << _mesh->mNumVertices << " vertices;" << std::endl;
    std::clog << "Mesh has: " << _mesh->mNumFaces << " faces; At most: " << _mesh->mNumFaces * 3 << " vertices;" << std::endl;
    std::clog << "Mesh has normals: " << std::boolalpha << _mesh->HasNormals() << std::endl;
 
    int primitives_count_old = primitives_count;
    
    // each face is a sigle triangle
    // should have nNumFaces of trinagles in total
    point3 vs[3];
    point3 ns[3];
    point2 uvs[3];

    for (unsigned int i = 0; i < _mesh->mNumFaces; i++) {
      const aiFace& face = _mesh->mFaces[i];
      for (unsigned int j = 0; j < 3; j++) {
        const int idx = face.mIndices[j];
        vs[j][0] = _mesh->mVertices[idx].x;
        vs[j][1] = _mesh->mVertices[idx].y;
        vs[j][2] = _mesh->mVertices[idx].z;
 
        if (_mesh->HasNormals()) {
          ns[j][0] = _mesh->mNormals[idx].x;
          ns[j][1] = _mesh->mNormals[idx].y;
          ns[j][2] = _mesh->mNormals[idx].z;
        }
              
        if (_mesh->HasTextureCoords(0)) {  // Check if there are UV coordinates
          uvs[j][0] = _mesh->mTextureCoords[0][idx].x;
          uvs[j][1] = _mesh->mTextureCoords[0][idx].y;
        }
      }
      primitives[primitives_count].v1 = vs[0];
      primitives[primitives_count].v2 = vs[1];
      primitives[primitives_count].v3 = vs[2];
      primitives[primitives_count].n1 = ns[0];
      primitives[primitives_count].n2 = ns[1];
      primitives[primitives_count].n3 = ns[2];
      primitives[primitives_count].uv1 = uvs[0];
      primitives[primitives_count].uv2 = uvs[1];
      primitives[primitives_count].uv3 = uvs[2];
      primitives[primitives_count].update_bounds();
      primitives_count++;
    }
    // process material
    aiMaterial* material = scene->mMaterials[_mesh->mMaterialIndex];
 
    auto mat = loadMaterial(material);
    
    for (int primitive_idx = primitives_count_old; primitive_idx < primitives_count; primitive_idx++) {
      primitives[primitive_idx].mat = mat;
    }
  }
 
  shared_ptr<material> loadMaterial(aiMaterial *mat) {
    const auto mat_name = string{mat->GetName().C_Str()};
    
    if (!materials_loaded.contains(mat_name)) {
      std::clog << "Loading material " << mat_name << std::endl;
      auto pbr_mat = make_shared<pbr>();
      pbr_mat->albedo = loadTexture(mat, aiTextureType_DIFFUSE);
      pbr_mat->emit = loadTexture(mat, aiTextureType_EMISSIVE);
      materials_loaded[mat_name] = pbr_mat;
    }
    return materials_loaded[mat_name];
  }
  
  shared_ptr<texture> loadTexture(aiMaterial *mat, aiTextureType type) {
    const aiString mat_name = mat->GetName();
    
    if (mat->GetTextureCount(type) == 0) { // no texture, use color instead
      if (type == aiTextureType_DIFFUSE) {
        aiColor4D diffuse_color;
        if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse_color)) {
          string key{"e_c"};
          key.append(std::to_string(diffuse_color.r));
          key.append(std::to_string(diffuse_color.g));
          key.append(std::to_string(diffuse_color.b));
          if (!textures_loaded.contains(key)) {
            textures_loaded[key] = make_shared<solid_color>(diffuse_color.r, diffuse_color.g, diffuse_color.b);
          }
          return textures_loaded[key];
        } else {
          std::clog << "ERROR: Failed to fetch color for diffuse material " << mat_name.C_Str() << std::endl;
          return default_diffuse;
        }
      }
      if (type == aiTextureType_EMISSIVE) {
        aiColor4D emission_color;
        if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &emission_color) && !emission_color.IsBlack()) {
          string key{"e_c"};
          key.append(std::to_string(emission_color.r));
          key.append(std::to_string(emission_color.g));
          key.append(std::to_string(emission_color.b));
          if (!textures_loaded.contains(key)) {
            textures_loaded[key] = make_shared<solid_color>(emission_color.r, emission_color.g, emission_color.b);
          }
          return textures_loaded[key];
        } else {
          std::clog << "ERROR: Failed to fetch color for emissive material " << mat_name.C_Str() << std::endl;
        }
        return default_emissive;
      }
      return nullptr;
    }

    aiString str;
    if (AI_SUCCESS == mat->GetTexture(type, 0, &str)) {
      std::string filename{str.C_Str()};
      filename = directory + '/' + filename;
      if (!textures_loaded.contains(filename)) {
        std::clog << "Loading texture at: " << filename << std::endl;
        textures_loaded[filename] = make_shared<image_texture>(filename.c_str());
      }
      return textures_loaded[filename];
    } else {
      std::clog << "ERROR: Failed to fetch texture path for " << mat_name.C_Str() << std::endl;
      switch(type) {
        case aiTextureType_DIFFUSE:
          return default_diffuse;
        case aiTextureType_EMISSIVE:
          return default_emissive;
        default:
          return nullptr;
      }
    }

//    aiTextureType_AMBIENT - don't need
//    aiTextureType_DIFFUSE - try texture, if no return checkers
//    aiTextureType_DIFFUSE_ROUGHNESS - try texture, if no return null
//    aiTextureType_EMISSIVE - try texture, if no return black texture
//    aiTextureType_EMISSION_COLOR - dunno
//    aiTextureType_NORMALS - try texture, if no return null
//    aiTextureType_METALNESS - try texture, if no return null
  }
};

#endif
