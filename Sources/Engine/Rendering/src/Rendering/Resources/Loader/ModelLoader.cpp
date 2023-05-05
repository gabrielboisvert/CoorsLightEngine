#include "Rendering/Resources/Loader/ModelLoader.h"
#include "Maths/Utils.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


using namespace Rendering::Resources::Loaders;

bool ModelLoader::importMesh(const char* pFilePath, std::vector<VK::VKMesh>& pMesh, std::map<std::string, Rendering::Data::BoneInfo>& pBoneInfoMap, Data::BoundingBox& pBox, int& pBoneCount)
{
    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    const aiScene* scene = importer.ReadFile(pFilePath, aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals |
                                                        aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices | 
                                                        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GenBoundingBoxes);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        if (scene != nullptr)
            if (scene->HasAnimations())
                return false;

        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return false;
    }

    pMesh.reserve(scene->mNumMeshes);

    processNode(scene->mRootNode, scene, pMesh, pBoneInfoMap, pBoneCount);
    
    //Process BoundingBox
    pBox = convertBoundingBox(scene->mMeshes[0]->mAABB);
    for (int i = 1; i < scene->mNumMeshes; i++)
    {
        Data::BoundingBox box = convertBoundingBox(scene->mMeshes[i]->mAABB);
        pBox.mMin = Maths::FVector3::vectorMin(pBox.mMin, box.mMin);
        pBox.mMax = Maths::FVector3::vectorMax(pBox.mMax, box.mMax);
    }
    pBox.mSize = pBox.mMax - pBox.mMin;

    return true;
}

void ModelLoader::processNode(const aiNode* pNode, const aiScene* pScene, std::vector<VK::VKMesh>& pMesh, std::map<std::string, Rendering::Data::BoneInfo>& pBoneInfoMap, int& pBoneCount)
{
    for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
    {
        aiMesh* mesh = pScene->mMeshes[pNode->mMeshes[i]];
        processMesh(mesh, pScene, pMesh, pBoneInfoMap, pBoneCount);
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++)
        processNode(pNode->mChildren[i], pScene, pMesh, pBoneInfoMap, pBoneCount);
}

void ModelLoader::processMesh(const aiMesh* pMesh, const aiScene* pScene, std::vector<VK::VKMesh>& pVKMesh, std::map<std::string, Rendering::Data::BoneInfo>& pBoneInfoMap, int& pBoneCount)
{
    std::vector<Rendering::Geometry::Vertex> vertices;
    std::vector<uint32_t> indices;

    //vertices
    for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
    {
        Rendering::Geometry::Vertex vertex;
        
        // position
        {
            Maths::FVector3 vector(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
            vertex.mPosition = vector;
        }

        // normals
        if (pMesh->HasNormals())
        {
            Maths::FVector3 vector(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);
            vertex.mNormals = vector;
        }

        // texture coordinates
        if (pMesh->mTextureCoords[0])
        {
            Maths::FVector2 vector(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
            vertex.mTexCoords = vector;
        }

        if (pMesh->HasTangentsAndBitangents())
        {
            // tangent
            {
                Maths::FVector3 vector(pMesh->mTangents[i].x, pMesh->mTangents[i].y, pMesh->mTangents[i].z);
                vertex.mTangent = vector;
            }

            // bitangent
            {
                Maths::FVector3 vector(pMesh->mBitangents[i].x, pMesh->mBitangents[i].y, pMesh->mBitangents[i].z);
                vertex.mBitangent = vector;
            }
        }

        vertices.push_back(vertex);
    }

    // faces
    for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
    {
        aiFace face = pMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    extractBoneWeightForVertices(vertices, pBoneInfoMap, pBoneCount, pMesh, pScene);

    pVKMesh.emplace_back(vertices, indices);
}

void ModelLoader::extractBoneWeightForVertices(std::vector<Rendering::Geometry::Vertex>& pVertices, std::map<std::string, Rendering::Data::BoneInfo>& pBoneInfoMap, int& pBoneCount, const aiMesh* pMesh, const aiScene* pScene)
{
    for (int boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = pMesh->mBones[boneIndex]->mName.C_Str();
        if (pBoneInfoMap.find(boneName) == pBoneInfoMap.end())
        {
            Rendering::Data::BoneInfo newBoneInfo;
            newBoneInfo.mId = pBoneCount;
            newBoneInfo.mOffset = convertMatrix(pMesh->mBones[boneIndex]->mOffsetMatrix);
            pBoneInfoMap[boneName] = newBoneInfo;
            boneID = pBoneCount;
            pBoneCount++;
        }
        else
            boneID = pBoneInfoMap[boneName].mId;

        if (boneID == -1)
            return;

        aiVertexWeight* weights = pMesh->mBones[boneIndex]->mWeights;
        int numWeights = pMesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            if (vertexId <= pVertices.size())
                setVertexBoneData(pVertices[vertexId], boneID, weight);
        }
    }
}

void ModelLoader::setVertexBoneData(Rendering::Geometry::Vertex& pVertex, int pBoneId, float pWeight)
{
    for (unsigned int i = 0; i < MAX_BONE_INFLUENCE; i++)
        if (pVertex.mBoneIDs[i] < 0)
        {
            pVertex.mWeights[i] = pWeight;
            pVertex.mBoneIDs[i] = pBoneId;
            break;
        }
}

Maths::FMatrix4 ModelLoader::convertMatrix(const aiMatrix4x4& pFrom)
{
    Maths::FMatrix4 to;
    to(0, 0) = pFrom.a1; to(1, 0) = pFrom.a2; to(2, 0) = pFrom.a3; to(3, 0) = pFrom.a4;
    to(0, 1) = pFrom.b1; to(1, 1) = pFrom.b2; to(2, 1) = pFrom.b3; to(3, 1) = pFrom.b4;
    to(0, 2) = pFrom.c1; to(1, 2) = pFrom.c2; to(2, 2) = pFrom.c3; to(3, 2) = pFrom.c4;
    to(0, 3) = pFrom.d1; to(1, 3) = pFrom.d2; to(2, 3) = pFrom.d3; to(3, 3) = pFrom.d4;
    return to;
}

Rendering::Data::BoundingBox ModelLoader::convertBoundingBox(const aiAABB& pFrom)
{
    return { {pFrom.mMin.x, pFrom.mMin.y, pFrom.mMin.z}, {pFrom.mMax.x, pFrom.mMax.y, pFrom.mMax.z} };
}