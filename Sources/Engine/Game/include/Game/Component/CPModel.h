#pragma once
#include "Game/Component/AComponent.h"
#include "Rendering/Resources/Model.h"
#include "Rendering/Resources/Material.h"

namespace Game::Component
{
	class CPModel : public AComponent
	{
	public:
		Rendering::Resources::Model* mModel = nullptr;
		std::string mName = "Empty";
		std::string mPath = "";

		Rendering::Resources::Material* mMat = nullptr;
#ifdef NSHIPPING
		Rendering::Resources::Material* mMatEditor = nullptr;
#endif
		std::string mNameMat = "Empty";
		std::string mPathMat = "";

		bool defaultMat = true;

		CPModel();
		AComponent* clone() override;
		~CPModel() override = default;

		void setModel(const std::string& pName, const char* pModel);
		void setModelWithPath(const char* pModel);
		void setModelWithPathLua(const char* pPath);
		Rendering::Resources::Model* getModel() const;


		void setMat(const std::string& pName, const char* pModel);
		void setMat(const char* pModel);
		void setMatWithPathLua(const char* pPath);
		Rendering::Resources::Material* getMat() const;

		void updateMat(Maths::FMatrix4& pVP, Maths::FMatrix4& pModel);
		void updateMatEditor(Maths::FMatrix4& pVP, Maths::FMatrix4& pModel);

		void draw(void* pCmd, Rendering::Data::Material* pPipeLine = nullptr);
		void drawEditor(void* pCmd, Rendering::Data::Material* pPipeLine = nullptr);

		void OnAwake() override {};
		void OnStart() override {};
		void OnDestroy() override {};
		void OnUpdate(float pDeltaTime) override {};

		void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) override;
	};
}