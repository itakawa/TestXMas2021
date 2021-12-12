# include <Siv3D.hpp> // OpenSiv3D v0.6.3
# include <Siv3D/EngineLog.hpp>

# include "PixieMesh.hpp"
# include "PixieCamera.hpp"
# include "LineString3D.hpp"

constexpr ColorF        BGCOLOR = { 0.8, 0.9, 1.0, 0 };
constexpr TextureFormat TEXFORMAT = TextureFormat::R8G8B8A8_Unorm_SRGB;
constexpr Size			WINDOWSIZE = { 1280, 768 };
constexpr StringView	APATH = U"../Asset/" ;
constexpr RectF			PIPWINDOW{ 900,512,370,240 } ;
constexpr ColorF        WHITE = ColorF{ 1,1,1,USE_COLOR };
constexpr Vec2			TREECENTER{ -100, -100 };
constexpr double		TREERASIUS = 100;
constexpr double		VOLALITY = 40;
constexpr double		TONAKAISPEED = 0.00003;

struct ActorRecord
{
	ColorF Color = WHITE;
	Float3 Pos = Float3 {0,0,0} ;
	Float3 Sca = Float3 {1,1,1} ;
	Float3 rPos = Float3 {0,0,0} ;
	Float3 eRot = Float3 {0,0,0} ;
	Quaternion qRot = Quaternion::Identity() ;
	Float3 eyePos = Float3 {0,0,0} ;
	Float3 focusPos =  Float3 {0,0,0};
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

Array<PixieMesh> pixieMeshes(NUMST);
PixieCamera cameraMain(WINDOWSIZE);

//トナカイ
void updateTonakai(Array<PixieMesh>& meshes, LineString3D& ls3, double & progressPos)
{
	static Array<Float3> prevpos(7);
	for (int32 i = 0; i < prevpos.size(); i++)
	{
		const double offset[7] = { 0, 0.0002,0.0002, 0.0004,0.0004, 0.0006,0.0006 };

		PixieMesh& mesh = meshes[ST_TONAKI_A + i];
		prevpos[i] = mesh.Pos;
		mesh.Pos = ls3.getCurvedPoint( progressPos+offset[i] );

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
	double progress = progressPos - 0.0004;
	if (progress < 0) progress += 1.0;

	Float3 prevpos = mesh.Pos;
	mesh.Pos = ls3.getCurvedPoint(progress);
	mesh.qRot = mesh.camera.getQLookAt(mesh.Pos, prevpos);
}

//トナカイカメラ
void updateCamera( PixieMesh& target, PixieMesh& mesh )
{
	mesh.camera.setFocusPosition(target.Pos);

	float speed = 0.01f;
	if (KeyRControl.pressed()) speed *= 5;
	if (KeyLeft.pressed())	mesh.rPos = mesh.camera.arcX(-speed).getEyePosition();
	if (KeyRight.pressed()) mesh.rPos = mesh.camera.arcX(+speed).getEyePosition();
	if (KeyUp.pressed())	mesh.rPos = mesh.camera.arcY(-speed).getEyePosition();
	if (KeyDown.pressed())	mesh.rPos = mesh.camera.arcY(+speed).getEyePosition();

	mesh.camera.updateView();
	mesh.camera.updateViewProj();
	mesh.setRotateQ(mesh.camera.getQForward());
}

void updateMainCamera(const PixieMesh& model, PixieCamera& camera)
{
	Float3 eyepos = camera.getEyePosition();
	Float3 focuspos = camera.getFocusPosition();

	Float2 delta = Cursor::DeltaF();
	Float3 vector = eyepos.xyz() - focuspos;
	Float2 point2D = Cursor::PosF();
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

	Mat4x4 mrot = Mat4x4::Identity();
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
	PixieMesh& meshSled    = pixieMeshes[ST_SLED]   = PixieMesh{ APATH + U"XMas.Sled.006.glb", Float3{0, 0, 0} };
	PixieMesh& meshFont    = pixieMeshes[ST_FONT]   = PixieMesh{ APATH + U"ToD4Font.005.glb",  Float3{0, 0, 0} };
	PixieMesh& meshCamera  = pixieMeshes[ST_CAMERA] = PixieMesh{ APATH + U"Camera.glb",        Float3{-50, 50, -50} };
	PixieMesh& meshTree    = pixieMeshes[ST_TREE]   = PixieMesh{ APATH + U"XMas.Tree.glb",     Float3{-100, 0, -100} };
	PixieMesh& meshGND     = pixieMeshes[ST_GND]    = PixieMesh{ APATH + U"XMas.GND.glb",      Float3{0, 0, 0} };
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

	meshTonakai.camera = PixieCamera(WINDOWSIZE, 45_deg, meshTonakai.Pos, meshTonakai.Pos + Float3{ 0,1,0 }, 0.05);
	meshCamera.camera  = PixieCamera(WINDOWSIZE, 45_deg, meshCamera.Pos, meshSled.Pos + Float3{ 0,0,0 }, 0.05);

	// 軌道計算
	ActorRecord val;
	for (int32 yy = 0; yy < 4; yy++)
	{
		for (int32 r = 0; r < 36; r++)
		{
			const Vec2 pos2 = TREECENTER + Circular(TREERASIUS-20*yy, ToRadians(r * 10));
			val.Pos = Float3{ pos2.x + VOLALITY *(Random()-0.5), 30*yy + VOLALITY *Random(), pos2.y + VOLALITY * (Random() - 0.5) };
			actorRecords.emplace_back( val );
		}
	}
	for (int32 yy = 4; yy >= 0; --yy)
	{
		for (int32 r = 0; r < 36; r++)
		{
			const Vec2 pos2 = TREECENTER + Circular(TREERASIUS-20*yy, ToRadians(r * 10));
			val.Pos = Float3{ pos2.x + VOLALITY * (Random() - 0.5), 30 * yy + VOLALITY * Random(), pos2.y + VOLALITY * (Random() - 0.5) };
			actorRecords.emplace_back(val);
		}
	}

	LineString3D lineString3D;
	for (int32 i = 1; i < actorRecords.size(); i++)
		lineString3D.emplace_back( actorRecords[i].Pos + actorRecords[i].rPos );

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
				updateMainCamera( meshGND, cameraMain );
				updateTonakai( pixieMeshes, lineString3D, progressPos );
				updateSled( meshSled, lineString3D, progressPos );
				updateCamera( meshSled, meshCamera );

				progressPos += TONAKAISPEED;
				if ( progressPos > 1.0 ) progressPos -= 1.0;

				//描画
				meshGND.drawMesh();
				meshTree.drawMesh();

				lineString3D.drawCatmullRom(actorRecords[0].Color);

				for (int32 i = 0; i < 7; i++) pixieMeshes[ST_TONAKI_A+i].drawAnime(0).nextFrame(0);
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

