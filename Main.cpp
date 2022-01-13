
# include <Siv3D.hpp> // OpenSiv3D v0.6.3
# include <Siv3D/EngineLog.hpp>

# include "PixieMesh.hpp"
# include "PixieCamera.hpp"
# include "LineString3D.hpp"

constexpr ColorF        BGCOLOR = { 0.0, 0.1, 0.5, 0 };
constexpr TextureFormat TEXFORMAT = TextureFormat::R8G8B8A8_Unorm_SRGB;
constexpr Size			WINDOWSIZE = { 1280, 768 };

//constexpr StringView	APATH = U"./Asset/";    // LINUX
constexpr StringView	APATH = U"../Asset/";   // WINDOWS

constexpr RectF			PIPWINDOW{ 900,512,370,240 };
constexpr ColorF        WHITE = ColorF{ 1,1,1,USE_COLOR };
constexpr Vec2			TREECENTER{ -100, -100 };
constexpr double		TREERASIUS = 100;
constexpr double		VOLALITY = 40;
constexpr double		TONAKAISPEED = 0.00008;
constexpr int32			NUMSNOW = 7 * 100;

struct ActorRecord
{
	ColorF Color = WHITE;
	Float3 Pos = Float3{ 0,0,0 };
	Float3 Sca = Float3{ 1,1,1 };
	Float3 rPos = Float3{ 0,0,0 };
	Float3 eRot = Float3{ 0,0,0 };
	Quaternion qRot = Quaternion::Identity();
	Float3 eyePos = Float3{ 0,0,0 };
	Float3 focusPos = Float3{ 0,0,0 };
};

enum SELECTEDTARGET
{
	ST_SLED, ST_FONT, ST_CAMERA, ST_TREE, ST_GND,
	ST_TONAKI_A,
	ST_TONAKI_B, ST_TONAKI_C, ST_TONAKI_D,
	ST_TONAKI_E, ST_TONAKI_F, ST_TONAKI_G,
	NUMST
};

Array<ActorRecord> actorRecords;

struct SnowFrake
{
	bool   exist = false;		                //結晶の有無
	Float3 prevpos = Float3{ 0,0,0 };	        //結晶軌道の位置情報
	double offsetD = 0.0;			            //結晶軌道の奥行位置
	double offsetH = 0.0;			            //結晶軌道の幅
	ColorF color = WHITE;			            //結晶の色
	Float3 scale = Float3{ 0,0,0 };				//結晶の大きさ
	Quaternion qRot = Quaternion::Identity();	//結晶の回転
	Quaternion qSpin = Quaternion::Identity();	//結晶のスピン
	double easing = 0.0;						//結晶のイージング(大きさと色に適用)
	double speed = 0.0;							//結晶のイージング速度
};

Array<SnowFrake> snowFrakes(NUMSNOW);

Array<PixieMesh> pixieMeshes(NUMST);
PixieCamera cameraMain(WINDOWSIZE);

void registerSnowFrake()
{
	for (int32 i = 0; i < NUMSNOW; i++)
	{
		if (snowFrakes[i].exist) continue;

		snowFrakes[i].prevpos = Float3{ 0,0,0 };
		snowFrakes[i].offsetD = 0.01;
		snowFrakes[i].offsetH = 2 * (Random(-1.0, +1.0));
		snowFrakes[i].color = WHITE;
		snowFrakes[i].scale = Float3{ Random(0.1,0.4), 1, Random(0.1,0.4) };
		snowFrakes[i].qRot = Quaternion::RollPitchYaw(Random() * Math::TauF, Random() * Math::TauF, Random() * Math::TauF);
		snowFrakes[i].qSpin = Quaternion::RollPitchYaw(Random() * 0.314, Random() * 0.314, Random() * 0.314);
		snowFrakes[i].easing = 0.0;
		snowFrakes[i].speed = Random(0.003, 0.006);
		snowFrakes[i].exist = true;
		break;
	}
}

template <typename I> double combineEase(I easeA, I easeB, double e, double separator)
{
	return (e < separator) ? std::invoke(easeA, e / separator) :
		1 - std::invoke(easeB, (e - separator) / (1 - separator));
}

//雪の結晶軌道
void updateSnowFrake(Array<PixieMesh>& meshes, LineString3D& ls3, double& progressPos)
{
	const double BASE[7] = { 0, 0.001,0.001, 0.002,0.002, 0.003,0.003 };

	for (int32 i = 0; i < 4; i++) registerSnowFrake();

	for (int32 i = 0; i < NUMSNOW; i++)
	{
		auto& snow = snowFrakes[i];
		if (snow.exist)
		{
			const String SNOW[6] = { U"\xBF",U"\xC0",U"\xC1",U"\xC2",U"\xC3",U"\xC4" };
			PixieMesh& mesh = meshes[ST_FONT];

			snow.easing += snow.speed;
			if (snow.easing > 1.0)
			{
				snow.exist = false;
				continue;
			}

			mesh.qRot = snow.qRot *= snow.qSpin;	//結晶の回転
			double pp = progressPos + BASE[i % 7] - snow.offsetD * EaseInSine(snow.easing);
			if (pp < 0.0) pp += 1.0;
			mesh.Pos = ls3.getCurvedPoint(pp);
			mesh.Sca = snow.scale;

			Float3 up{ 0,1,0 };
			Float3 right{ 0,0,0 };
			mesh.camera.setEyePosition(snow.prevpos).setFocusPosition(mesh.Pos)
				.getQLookAt(mesh.Pos, snow.prevpos, &up, &right);
			snow.prevpos = mesh.Pos;
			mesh.Pos += right * snow.offsetH;								//右ベクトルから幅に適用

			double v = combineEase(EaseInSine, EaseInSine, snow.easing, 0.8);
			mesh.setScale(Float3{ v,v,v });

			mesh.drawString(SNOW[i % 6], 0, 0, snow.color);
		}
	}
}

//トナカイ
void updateTonakai(Array<PixieMesh>& meshes, LineString3D& ls3, double& progressPos)
{
	static Array<Float3> prevpos(7);
	for (uint32 i = 0; i < prevpos.size(); i++)
	{
		const double offset[7] = { 0, 0.001,0.001, 0.002,0.002, 0.003,0.003 };

		PixieMesh& mesh = meshes[ST_TONAKI_A + i];
		prevpos[i] = mesh.Pos;
		mesh.Pos = ls3.getCurvedPoint(progressPos + offset[i]);

		Float3 up{ 0,1,0 };
		Float3 right{ 0,0,0 };
		mesh.qRot = mesh.camera.getQLookAt(mesh.Pos, prevpos[i], &up, &right);
		if (i == 1 || i == 3 || i == 5) mesh.rPos = -right;
		if (i == 2 || i == 4 || i == 6) mesh.rPos = +right;
	}

	if (progressPos >= 1) progressPos = 0;
}

//ソリ
void updateSled(PixieMesh& mesh, LineString3D& ls3, double& progressPos)
{
	double progress = progressPos - 0.004;
	if (progress < 0) progress += 1.0;

	Float3 prevpos = mesh.Pos;
	mesh.Pos = ls3.getCurvedPoint(progress);
	mesh.qRot = mesh.camera.getQLookAt(mesh.Pos, prevpos);
}

//トナカイカメラ
void updateCamera(PixieMesh& tonakai, PixieMesh& camera)
{
	tonakai.camera.setFocusPosition(tonakai.Pos);

	float speed = 0.01f;
	if (KeyRControl.pressed()) speed *= 5;
	if (KeyLeft.pressed())	tonakai.camera.m_eyePos = tonakai.Pos + camera.camera.arcX(-speed).getEyePosition();
	if (KeyRight.pressed()) tonakai.camera.m_eyePos = tonakai.Pos + camera.camera.arcX(+speed).getEyePosition();
	if (KeyUp.pressed())	tonakai.camera.m_eyePos = tonakai.Pos + camera.camera.arcY(-speed).getEyePosition();
	if (KeyDown.pressed())	tonakai.camera.m_eyePos = tonakai.Pos + camera.camera.arcY(+speed).getEyePosition();
	camera.Pos = tonakai.camera.m_eyePos;
	camera.setRotateQ(tonakai.camera.getQForward());

	tonakai.camera.updateView();
	tonakai.camera.updateViewProj();
}

void updateMainCamera(const PixieMesh& model, PixieCamera& camera)
{
	Float3 eyepos = camera.getEyePosition();
	Float3 focuspos = camera.getFocusPosition();

	Float2 delta = Cursor::DeltaF();
	Float3 distance;

	float speedM = 1;
	if (KeyLControl.down() || MouseM.down())								//中ボタンドラッグ：回転
	{
		const Ray mouseRay = camera.screenToRay(Cursor::PosF());
		if (const auto depth = mouseRay.intersects(model.ob))
		{
			Float3 point3D = mouseRay.point_at(*depth);						//ポイントした3D座標を基点
			distance = point3D - eyepos;

			Float3 identity = distance.normalized();
			speedM = distance.length() / identity.length() / 100;
		}
	}

	if (KeyLControl.pressed() || MouseM.pressed())
	{
		bool rev = camera.dolly((float)Mouse::Wheel() * speedM / 5, true);	//中ボタンウィール：拡縮
		if (rev)
		{
			rev = camera.setEyePosition(eyepos).dolly((float)Mouse::Wheel() * speedM / 100, true);
			if (rev)
			{
				rev = camera.setEyePosition(eyepos).dolly((float)Mouse::Wheel() * speedM / 1000, true);
				if (rev) camera.setEyePosition(eyepos);
			}
		}
	}

	if (MouseR.pressed())							//右ボタンドラッグ：平行移動
	{
		if (KeyLShift.pressed()) speedM *= 5;		//Shift押下で5倍速
		camera.trackX(delta.x * speedM / 10);
		camera.craneY(delta.y * speedM / 10);
	}

	//	Mat4x4 mrot = Mat4x4::Identity();
	if (MouseM.pressed())
	{
		Float4 ve = { 0,0,0,0 };  // 視点移動量
		Float3 vf = { 0,0,0 };    // 注視点移動量
		if (KeyLShift.pressed())					// Shiftで5倍速
		{
			speedM = 5;
			ve *= speedM;
			vf *= speedM;
		}
		camera.arcX(delta.x * speedM / 100);
		camera.arcY(delta.y * speedM / 100);
		camera.setUpDirection(Float3{ 0,1,0 });
	}

	float speedK = camera.getBasisSpeed() / 100 * 5;
	if (KeyLShift.pressed()) speedK *= 10;			//Shift押下で5倍速

	if (KeyW.pressed()) camera.dolly(+speedK, true);
	if (KeyA.pressed()) camera.trackX(+speedK);
	if (KeyS.pressed()) camera.dolly(-speedK, true);
	if (KeyD.pressed()) camera.trackX(-speedK);

	if (KeyE.pressed()) camera.craneY(+speedK);
	if (KeyX.pressed()) camera.craneY(-speedK);
	if (KeyQ.pressed()) camera.tilt(+speedK / 100);
	if (KeyZ.pressed()) camera.tilt(-speedK / 100);

	camera.setUpDirection(Float3{ 0,1,0 });
}

void Main()
{
	//ウィンドウ初期化
	Window::Resize(WINDOWSIZE);
	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetBackground(BGCOLOR);

	//描画レイヤ(レンダーテクスチャ)初期化
	static MSRenderTexture rtexMain = { (unsigned)WINDOWSIZE.x, (unsigned)WINDOWSIZE.y, TEXFORMAT, HasDepth::Yes };
	static MSRenderTexture rtexSub = { (unsigned)WINDOWSIZE.x, (unsigned)WINDOWSIZE.y, TEXFORMAT, HasDepth::Yes };

	//メッシュ設定
	PixieMesh& meshSled = pixieMeshes[ST_SLED] = PixieMesh{ APATH + U"XMas.Sled.006.glb", Float3{0, 0, 0} };
	PixieMesh& meshFont = pixieMeshes[ST_FONT] = PixieMesh{ APATH + U"ToD4Font.005.glb",  Float3{0, 0, 0} };
	PixieMesh& meshCamera = pixieMeshes[ST_CAMERA] = PixieMesh{ APATH + U"Camera.glb",    Float3{-50, 50, -50} };
	PixieMesh& meshTree = pixieMeshes[ST_TREE] = PixieMesh{ APATH + U"XMas.Tree.glb",     Float3{-100, 0, -100} };
	PixieMesh& meshGND = pixieMeshes[ST_GND] = PixieMesh{ APATH + U"XMas.GND.glb",        Float3{0, 0, 0} };
	PixieMesh& meshTonakai = pixieMeshes[ST_TONAKI_A];

	for (int32 i = 0; i < 7; i++)
		pixieMeshes[ST_TONAKI_A + i] = PixieMesh{ APATH + U"XMas.Tonakai.006.glb", Float3{1, 0, 0} };

	//メッシュ初期化
	meshSled.initModel(MODELANI, WINDOWSIZE, NOTUSE_STRING, USE_MORPH, nullptr, HIDDEN_BOUNDBOX, 60, 0);
	meshFont.initModel(MODELNOA, WINDOWSIZE, USE_STRING, USE_MORPH);
	meshTree.initModel(MODELNOA, WINDOWSIZE, USE_STRING, USE_MORPH);
	meshCamera.initModel(MODELNOA, WINDOWSIZE);
	meshGND.initModel(MODELNOA, WINDOWSIZE);

	for (int32 i = 0; i < 7; i++)
		pixieMeshes[ST_TONAKI_A + i].initModel(MODELANI, WINDOWSIZE, NOTUSE_STRING, USE_MORPH, nullptr, HIDDEN_BOUNDBOX, 30, 0);

	//カメラ初期化
	Float4 eyePosMain = { 0, 1, 30.001, 0 };		//視点 XYZは座標、Wはカメラロールをオイラー角で保持
	Float3 focusPosMain = { 0,  0, 0 };
	cameraMain = PixieCamera(WINDOWSIZE, 45_deg, eyePosMain.xyz(), focusPosMain, 0.05);

	meshTonakai.camera = PixieCamera(WINDOWSIZE, 45_deg, meshTonakai.Pos, meshTonakai.Pos + Float3{ 0,10,0 }, 0.05);
	//	meshCamera.camera  = PixieCamera(WINDOWSIZE, 45_deg, meshCamera.Pos, meshSled.Pos + Float3{ 0,0,0 }, 0.05);

    // 軌道計算
	ActorRecord val;
	for (int32 yy = 0; yy < 4; yy++)
	{
		for (int32 r = 0; r < 18; r++)
		{
			const Vec2 pos2 = TREECENTER + Circular(TREERASIUS, ToRadians(r * 20));
			val.Pos = Float3{ pos2.x + VOLALITY * (Random() - 0.5),
							  30 * yy + VOLALITY * Random(),
							  pos2.y + VOLALITY * (Random() - 0.5) };
			actorRecords.emplace_back(val);
		}
	}

	LineString3D lineString3D;
	for (uint32 i = 1; i < actorRecords.size(); i++)
		lineString3D.emplace_back(actorRecords[i].Pos + actorRecords[i].rPos);

	double progressPos = 0;	//現在位置をスタート位置に設定

	while (System::Update())
	{
		//メインレイヤ描画
		{
			const ScopedRenderTarget3D rtmain{ rtexMain.clear(BGCOLOR) };

			Graphics3D::SetCameraTransform(cameraMain.getViewProj(), cameraMain.getEyePosition());
			Graphics3D::SetGlobalAmbientColor(ColorF{ 1.0 });
			Graphics3D::SetSunColor(ColorF{ 1.0 });
			{
				const ScopedRenderStates3D rs{ SamplerState::RepeatAniso, RasterizerState::SolidCullFront };

				//制御
				updateMainCamera(meshGND, cameraMain);
				updateTonakai(pixieMeshes, lineString3D, progressPos);
				updateSnowFrake(pixieMeshes, lineString3D, progressPos);
				updateSled(meshSled, lineString3D, progressPos);
				updateCamera(meshSled, meshCamera);

				progressPos += TONAKAISPEED;
				if (progressPos > 1.0) progressPos -= 1.0;

				//描画
				meshGND.drawMesh();
				meshTree.drawMesh();

				static bool hiddenLine = false;
				if (KeyPause.pressed()) hiddenLine = !hiddenLine;
				if( hiddenLine ) lineString3D.drawCatmullRom(actorRecords[0].Color);

				for (int32 i = 0; i < 7; i++) pixieMeshes[ST_TONAKI_A + i].drawAnime(0).nextFrame(0);
				meshSled.drawAnime(0).nextFrame(0);
				meshCamera.drawMesh();
			}

			Graphics3D::Flush();
			rtexMain.resolve();
			Shader::LinearToScreen(rtexMain);

			//サブレイヤ描画
			{
				const ScopedRenderTarget3D rtsub{ rtexSub.clear(BGCOLOR) };
				const ScopedRenderStates3D rs{ SamplerState::RepeatAniso, RasterizerState::SolidCullFront };

				Graphics3D::SetCameraTransform(meshCamera.camera.getViewProj(), meshCamera.camera.getEyePosition());
				Graphics3D::SetGlobalAmbientColor(ColorF{ 1.0 });
				Graphics3D::SetSunColor(ColorF{ 1.0 });

				meshGND.drawMesh();
				for (int32 i = 0; i < 7; i++) pixieMeshes[ST_TONAKI_A + i].drawAnime(0);
				meshSled.drawAnime(0);

				Graphics3D::Flush();
				rtexSub.resolve();
				Shader::LinearToScreen(rtexSub, PIPWINDOW);
			}

		}
	}
}

