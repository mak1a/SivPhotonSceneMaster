#include "SivPhotonSceneMaster.hpp"
# include "ENCRYPTED_PHOTON_APP_ID.SECRET"

using MyScene = NetworkSystem::SivPhotonSceneMaster<String>;

class Title : public MyScene::Scene {
private:
	// 変数の定義など

	void connectReturn(int32 errorCode, const String& errorString, const String& region, const String& cluster) override
	{
		Print << U"ok";
	}

public:
	// コンストラクタ
	Title(const InitData& init_) : IScene(init_)
	{
		this->connect(U"Siv");
	}

	// 更新関数
	void update() override
	{

	}

	// 描画関数(const 修飾)
	void draw() const override
	{

	}
};

void Main()
{
	const std::string encryptedAppID{ SIV3D_OBFUSCATE(ENCRYPTED_PHOTON_APP_ID) };
	const String appid = Unicode::WidenAscii(encryptedAppID);
	MyScene manager(appid, U"1.0");
	manager.add<Title>(U"Title");

	while (s3d::System::Update())
	{
		if (!manager.update()) break;
	}
}
