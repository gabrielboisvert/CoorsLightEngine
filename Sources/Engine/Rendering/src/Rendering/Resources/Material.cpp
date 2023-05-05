#pragma once
#include "Rendering/Resources/Material.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "EngineCore/ResourceManagement/ResourceManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Game/SceneSys/SceneManager.h"

#ifdef NSHIPPING
    #include "Editor/Widget/WidgetEditor.h"
    #include "Editor/Utils/Utils.h"
#endif

using namespace Rendering::Resources;

Material::Material(const std::string& pFileName) : mUnibuffer(VK_SHADER_STAGE_VERTEX_BIT), mFilename(pFileName)
{
    update();
}

Material::~Material()
{
    for (Material* mat : mInstance)
        delete mat;
}

void Material::bindMat(Rendering::Data::Material& pPipeLine)
{
    if (!ready)
        return;

    pPipeLine.bindDescriptor("texAlbedo", mTextureAlbedo->mTextureSets);
    pPipeLine.bindDescriptor("texNormal", mTextureNormal->mTextureSets);
    pPipeLine.bindDescriptor("texMetallic", mTextureMetallic->mTextureSets);
    pPipeLine.bindDescriptor("texRoughness", mTextureRoughness->mTextureSets);
    pPipeLine.bindDescriptor("texAO", mTextureAO->mTextureSets);
    pPipeLine.bindDescriptor("ubo", mUnibuffer.mDescriptorSets);
}

void Material::setModelData(Maths::FMatrix4& pVP, Maths::FMatrix4& pModel)
{
    mUnibuffer.mData.mViewProjection = pVP;
    mUnibuffer.mData.mModel = pModel;

    mUnibuffer.updateData();
}

void Material::update()
{
    //Read json
    std::ifstream file(mFilename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    rapidjson::Document doc;
    bool result = doc.Parse(buffer.str().c_str()).HasParseError();
    if (result)
        return;

    

    //Color
    auto obj = doc["Result"]["color"].GetArray();
    mUnibuffer.mData.mColor = Maths::FVector4(obj[0].GetInt() / 255.f, obj[1].GetInt() / 255.f, obj[2].GetInt() / 255.f, obj[3].GetInt() / 255.f);

    //Texture Albedo
    {
        //Bool
        mUnibuffer.mData.mHasTextureAlbedo = (int)doc["Result"]["HasTexture"].GetBool();

        std::string str = doc["Result"]["TextureAlbedo"].GetString();
        if (mUnibuffer.mData.mHasTextureAlbedo && !doc["Result"]["DefaultTexture"].GetBool())
        {
#ifdef NSHIPPING
            str = Editor::Utils::qStringToStdString(service(Editor::Widget::WidgetEditor).mLocation.mFolder) + "/" + str;
#else
            str = service(Game::SceneSys::SceneManager).mProjectPath + "/" + str;
#endif
        }

        {
            static std::mutex mutex;
            std::unique_lock lock(mutex);
            mTextureAlbedo = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Texture>(str.c_str(), str);
        }
    }

    //Texture Normal
    {
        mTextureNormal = loadTexture(doc["Result"]["TextureNormal"]);
        mUnibuffer.mData.mHasTextureNormal = (int)(mTextureNormal != mTextureAlbedo);
    }

    //Texture Metallic
    {
        mTextureMetallic = loadTexture(doc["Result"]["TextureMetallic"]);
        mUnibuffer.mData.mHasTextureMetalic = (int)(mTextureMetallic != mTextureAlbedo);
    }

    //Texture Roughness
    {
        mTextureRoughness = loadTexture(doc["Result"]["TextureRoughness"]);
        mUnibuffer.mData.mHasTextureRoughness = (int)(mTextureRoughness != mTextureAlbedo);
    }

    //Texture AO
    {
        mTextureAO = loadTexture(doc["Result"]["TextureAO"]);
        mUnibuffer.mData.mHasAO = (int)(mTextureAO != mTextureAlbedo);
    }

    updateInstance();

    ready = true;
}

Rendering::Resources::Texture* Material::loadTexture(rapidjson::Value& pValue)
{
    std::string str = pValue.GetString();
    if (!str.empty())
    {
#ifdef NSHIPPING
        str = Editor::Utils::qStringToStdString(service(Editor::Widget::WidgetEditor).mLocation.mFolder) + "/" + str;
#else
        str = service(Game::SceneSys::SceneManager).mProjectPath + "/" + str;
#endif
    }
    else
        return mTextureAlbedo;
        
    return service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Texture>(str.c_str(), str);
}

void Material::updateInstance()
{
    for (Material* mat : mInstance)
    {
        mat->mUnibuffer.mData.mHasTextureAlbedo = this->mUnibuffer.mData.mHasTextureAlbedo;
        mat->mUnibuffer.mData.mColor = this->mUnibuffer.mData.mColor;
        mat->mTextureAlbedo = this->mTextureAlbedo;
        mat->mTextureNormal = this->mTextureNormal;
        mat->mTextureMetallic = this->mTextureMetallic;
        mat->mTextureRoughness = this->mTextureRoughness;
        mat->mTextureAO = this->mTextureAO;
    }
}

Material* Material::getInstance()
{
    Material* instance = new Material(mFilename);
    instance->previewCreated = this->previewCreated;
    mInstance.push_back(instance);
    return instance;
}