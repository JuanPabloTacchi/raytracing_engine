#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Objeto {
    unsigned int baseVertex; // Dónde empiezan sus puntos en el array global
    unsigned int numIndices;  // Cuántos índices tiene en el array global
};

std::vector<Objeto> listaObjetos;
std::vector<float> vertexData; // Formato: X, Y, Z, R, G, B
std::vector<unsigned int> indicesData;




Assimp::Importer importer;

// Leemos la escena con flags de post-procesamiento (triangular caras, generar normales, etc.)
const aiScene* scene = importer.ReadFile("mi_escena.blend", 
    aiProcess_Triangulate | 
    aiProcess_FlipUVs | 
    aiProcess_GenSmoothNormals);

// Verificación de errores
if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cout << "Error de Assimp: " << importer.GetErrorString() << std::endl;
    return;
}

// Ahora puedes acceder a los datos
std::cout << "Número de mallas: " << scene->mNumMeshes << std::endl;


void extraerDatosBlender(const aiScene* scene) {
    unsigned int offsetVertices = 0;

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];
        
        // 1. Guardar metadatos del objeto
        Objeto obj;
        //obj.nombre = mesh->mName.C_Str();
        obj.baseVertex = offsetVertices;
        obj.numIndices = mesh->mNumFaces * 3;
        listaObjetos.push_back(obj);

        // 2. Extraer Atributos de Vértices (XYZ + RGB)
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
            // Posiciones (X, Y, Z)
            vertexData.push_back(mesh->mVertices[v].x);
            vertexData.push_back(mesh->mVertices[v].y);
            vertexData.push_back(mesh->mVertices[v].z);

            // Colores (R, G, B)
            // Blender no siempre exporta colores de vértices. Si no existen, ponemos blanco (1.0).
            if (mesh->HasVertexColors(0)) {
                vertexData.push_back(mesh->mColors[0][v].r);
                vertexData.push_back(mesh->mColors[0][v].g);
                vertexData.push_back(mesh->mColors[0][v].b);
            } else {
                vertexData.push_back(1.0f); vertexData.push_back(1.0f); vertexData.push_back(1.0f);
            }
        }

        // 3. Extraer Índices (Triángulos)
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];
            for (unsigned int i = 0; i < face.mNumIndices; ++i) {
                // Sumamos el offset para que el índice sea global respecto al array de vértices
                indicesData.push_back(face.mIndices[i] + offsetVertices);
            }
        }

        // Actualizar el offset para el siguiente objeto
        offsetVertices += mesh->mNumVertices;
    }
}