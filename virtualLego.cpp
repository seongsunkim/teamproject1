///////////// ///////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <d3dx9core.h>
#include <string>
#include <random>
#include <iostream>

IDirect3DDevice9* Device = NULL;

// window size
const int Width = 1024;
const int Height = 720;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)
//float spherePos[12][2] = { {-2.7f,0} , {2.4f,0} , {4.0f,-2.5f} , {1.0f, 1.0f}, {-1.0f, -1.0f}, {1.0f,1.5f},{-1.0f,1.5f},{1.0f,-1.5f},{-1.0f,-1.5f}, {1.5f,1.5f}, {-1.5f, -1.5f}, {-2.7f,-0.9f} };
float** spherePos = NULL;
//첫 번째가 Paddle, 맨 끝이 Bullet
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[4] = { d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE};

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
protected:
	float center_x, center_y, center_z;
	float m_radius;
	float m_velocity_x;
	float m_velocity_z;
	float distance(CSphere& ball) {
		float dx = center_x - ball.center_x;
		float dz = center_z - ball.center_z;

		return sqrt(dx * dx + dz * dz);
	}

public:
	CSphere(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
		m_pSphereMesh = NULL;
	}
	~CSphere(void) {}

public:
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
			return false;
		return true;
	}

	void destroy(void)
	{
		if (m_pSphereMesh != NULL) {
			m_pSphereMesh->Release();
			m_pSphereMesh = NULL;
		}
	}

	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball)
	{
		return distance(ball) < getRadius() + ball.getRadius();
	}

	virtual void hitBy(CSphere& ball) = 0;

	virtual void ballUpdate(float timeDiff) = 0;

	virtual boolean isRemoving() = 0;

	double getVelocity_X() { return this->m_velocity_x; }
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x = x;	center_y = y;	center_z = z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getRadius(void)  const { return (float)(M_RADIUS); }
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	D3DXVECTOR3 getCenter(void) const
	{
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pSphereMesh;
};

class Life {
public:
private:
	int lifeCount;
	

public:
	Life() { lifeCount = 3; }
	void decrease() {
		lifeCount--;
	}

	int getLife() {
		return lifeCount;
	}//무슨 역할이였지

	boolean isDead() {
		cout << "life 다 사용\n";
		return lifeCount <= 0;
	}
};

class Paddle : public CSphere {
private:
	float mouseZ = 0;
public:

	virtual void hitBy(CSphere& ball) {
	}

	virtual void ballUpdate(float timeDiff) {
		if (-2.75 <= mouseZ && mouseZ <= 2.75) {
			setCenter(center_x, center_y, mouseZ);
		}
	}

	virtual boolean isRemoving() {
		return false;
	}

	void mouseMoved(float z) {
		mouseZ = z;
	}
};

enum BulletState {
	Waiting,
	InScreen
};// => 초기 상태: Waiting, InScreen 상태에서 bullet이 벽의 범위를 벗어나면 다시 Waiting 상태로 변하게 했음

class Bullet : public CSphere {
private:
	Life& life;
	Paddle& paddle;
	BulletState currentState;
	boolean isSpace;

public:
	Bullet(Life& life, Paddle& paddle) : life(life), paddle(paddle) { currentState = Waiting; isSpace = true; }

	void hitBy(CSphere& ball) {	//block과 paddle이 인자로 들어옴
		if (hasIntersected(ball)) {
			D3DXVECTOR3 dx = ball.getCenter() - getCenter();
			float d = distance(ball);
			D3DXVECTOR3 normalized_dx = dx / d;
			D3DXVECTOR3 v(getVelocity_X(), 0, getVelocity_Z());
			float dotproduct = D3DXVec3Dot(&v, &normalized_dx);
			if (dotproduct > 0) {
				D3DXVECTOR3 reflected = v - 2.0f * dotproduct * normalized_dx;
				setPower(reflected.x, reflected.z);
			}
		}
	}
	void shootPressed() {
		// 총알 발사 시 InScreen 상태로 전환
		isSpace = false;
		setPower(2.0, 0.0);

		currentState = InScreen;
	}

	void ballUpdate(float timeDiff)//현재 bullet의 상태 업데이트
	{
		if (currentState == Waiting) {
			// Waiting 상태에서 paddle 위치 따라가기
			// paddle의 바로 위로 위치 설정
			D3DXVECTOR3 center = paddle.getCenter();
			setCenter(center.x + getRadius() * 2, center.y, center.z);

		}
		else if (currentState == InScreen) {
			const float TIME_SCALE = 3.3;
			D3DXVECTOR3 cord = this->getCenter();
			double vx = abs(this->getVelocity_X());
			double vz = abs(this->getVelocity_Z());

			if (vx > 0.01 || vz > 0.01)
			{
				float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
				float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

				if (tX >= (4.5 - M_RADIUS)) {
					tX = 4.5 - M_RADIUS;
					m_velocity_x = -m_velocity_x; // 벽에 부딪혔을 때 반사
				}
				else if (tX <= (-4.5 + M_RADIUS)) {
					//tX = -4.5 + M_RADIUS;
					//m_velocity_x = -m_velocity_x; // 벽에 부딪혔을 때 반사

					currentState = Waiting;
					isSpace = true;
					//벽의 X축의 마이너스 방향으로 벗어나면 Waiting 상태로 변경됌
					life.decrease();
				}

				if (tZ <= (-3 + M_RADIUS)) {
					tZ = -3 + M_RADIUS;
					m_velocity_z = -m_velocity_z; // 벽에 부딪혔을 때 반사
				}
				else if (tZ >= (3 - M_RADIUS)) {
					tZ = 3 - M_RADIUS;
					m_velocity_z = -m_velocity_z; // 벽에 부딪혔을 때 반사
				}
				this->setCenter(tX, cord.y, tZ);
			}
			else { this->setPower(0, 0); }
			//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
			/*double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400;
			if (rate < 0)
				rate = 0;
			this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);*/
		}
		// Csphere에서 가져옴
		//속도가 느려지는 것 수정함
	}
	//Csphere에서 가져옴

	boolean isRemoving() {
		return false;
	}//bullet은 공이 없어지지 않으므로 항상 false

	boolean getIsSpace() {
		return isSpace;
	}

};

class Point {
private:
	int pointCount;
public:
	Point() { pointCount = 0; }
	void increase() {
		pointCount++;
	}
	int getPoint() {
		return pointCount;
	}
};

class Block : public CSphere {
private:
	bool _isRemoving = false;
	Point& point;

public:
	Block(Point& point) : point(point) {};

	void hitBy(CSphere& ball) // bullet이 인자로 들어옴
	{
		if (hasIntersected(ball)) {
			_isRemoving = true;
		}
	}

	void ballUpdate(float timeDiff) {
		if (isRemoving()) {
			this->setPower(0, 0);
			this->setCenter(0, 0, 0);

			point.increase();
			//포인트 증가
		}
	}

	boolean isRemoving() {
		return _isRemoving;
	}
};


// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

	float					m_x;
	float					m_z;
	float                   m_width;
	float                   m_depth;
	float					m_height;

public:
	CWall(void)
	{
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL;
	}
	~CWall(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false;

		m_mtrl.Ambient = color;
		m_mtrl.Diffuse = color;
		m_mtrl.Specular = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;

		m_width = iwidth;
		m_depth = idepth;

		if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
			return false;
		return true;
	}
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release();
			m_pBoundMesh = NULL;
		}
	}
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld);
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
		pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
	}

	bool hasIntersected(CSphere& ball) {
		D3DXVECTOR3 ballCenter = ball.getCenter();
		float ballRadius = ball.getRadius();

		// 벽의 위치 및 치수를 얻기
		D3DXVECTOR3 wallCenter(m_x, 0.0f, m_z);  // 벽은 Y=0, 즉 XZ평면에 있다. 공도 마찬가지로 XZ평면에 있음
		//m_x:벽의 중심 X좌표
		//m_x:벽의 중심 Z좌표
		//ex)  (3,0.5) -> 벽이 3,0,5의 위치에 있다 => 벽의 위치는 

		//m_width, m_depth, m_height -> 벽의 가로, 깊이, 높이

		// 3D 공간에서 충돌을 확인
		bool collisionX = (ballCenter.x + ballRadius >= wallCenter.x - m_width / 2) &&
			(ballCenter.x - ballRadius <= wallCenter.x + m_width / 2);
		//벽의 X 범위 내에 공이 위치하는지 판단하는 알고리즘

		bool collisionZ = (ballCenter.z + ballRadius >= wallCenter.z - m_depth / 2) &&
			(ballCenter.z - ballRadius <= wallCenter.z + m_depth / 2);
		//같은 원리

		bool collisionY = (ballCenter.y + ballRadius >= wallCenter.y - m_height / 2) &&
			(ballCenter.y - ballRadius <= wallCenter.y + m_height / 2);
		//같은 원리

		return collisionX && collisionZ && collisionY;
	}

	void hitBy(CSphere& ball) {
		if (hasIntersected(ball)) {
			// 벽에 부딪혔을 때 벽의 법선벡터로 공이 벽에 입사하는 반사각으로 공을 반사
			D3DXVECTOR3 wallNormal(0.0f, 1.0f, 0.0f); // 이를위한 수직벡터, 벽은 XZ 평면에 있음

			// 법선 벡터를 사용한 속도 계산
			D3DXVECTOR3 incidentVelocity(ball.getVelocity_X(), 0.0f, ball.getVelocity_Z());
			D3DXVECTOR3 reflectedVelocity = incidentVelocity - 2.0f * D3DXVec3Dot(&incidentVelocity, &wallNormal) * wallNormal;

			//공의 속도 다시 설정
			ball.setPower(reflectedVelocity.x, reflectedVelocity.z);
		}
	}

	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getHeight(void) const { return M_HEIGHT; }

private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }

	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pBoundMesh;

};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
	CLight(void)
	{
		static DWORD i = 0;
		m_index = i++;
		D3DXMatrixIdentity(&m_mLocal);
		::ZeroMemory(&m_lit, sizeof(m_lit));
		m_pMesh = NULL;
		m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_bound._radius = 0.0f;
	}
	~CLight(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
	{
		if (NULL == pDevice)
			return false;
		if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
			return false;

		m_bound._center = lit.Position;
		m_bound._radius = radius;

		m_lit.Type = lit.Type;
		m_lit.Diffuse = lit.Diffuse;
		m_lit.Specular = lit.Specular;
		m_lit.Ambient = lit.Ambient;
		m_lit.Position = lit.Position;
		m_lit.Direction = lit.Direction;
		m_lit.Range = lit.Range;
		m_lit.Falloff = lit.Falloff;
		m_lit.Attenuation0 = lit.Attenuation0;
		m_lit.Attenuation1 = lit.Attenuation1;
		m_lit.Attenuation2 = lit.Attenuation2;
		m_lit.Theta = lit.Theta;
		m_lit.Phi = lit.Phi;
		return true;
	}
	void destroy(void)
	{
		if (m_pMesh != NULL) {
			m_pMesh->Release();
			m_pMesh = NULL;
		}
	}
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return false;

		D3DXVECTOR3 pos(m_bound._center);
		D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
		D3DXVec3TransformCoord(&pos, &pos, &mWorld);
		m_lit.Position = pos;

		pDevice->SetLight(m_index, &m_lit);
		pDevice->LightEnable(m_index, TRUE);
		return true;
	}

	void draw(IDirect3DDevice9* pDevice)
	{
		if (NULL == pDevice)
			return;
		D3DXMATRIX m;
		D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
		pDevice->SetTransform(D3DTS_WORLD, &m);
		pDevice->SetMaterial(&d3d::WHITE_MTRL);
		m_pMesh->DrawSubset(0);
	}

	D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh* m_pMesh;
	d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
vector<CSphere*> g_sphere;
CLight	g_light;
Point g_point;
Life g_life;
Paddle* g_paddle;
ID3DXFont* g_pFont = NULL;
random_device rd;
mt19937 gen(rd());

double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

void destroyAllLegoBlock(void)
{
}

bool InitFont()
{
	if (FAILED(D3DXCreateFontW(Device, 20, 0, FW_BLACK, 1, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Arial", &g_pFont)))
	{
		return false;
		// 오류 처리
	}


	return true;
}

bool isOverlap(float x1, float z1, float x2, float z2) {
	float distance = std::sqrt(std::pow(x2 - x1, 2) + std::pow(z2 - z1, 2));
	return distance < 2 * M_RADIUS;
}

bool initializeSpheres() {

	uniform_int_distribution<int> distributionInt(10, 20);
	int randomRow = distributionInt(gen);
	//int randomRow = 12;

	std::uniform_real_distribution<float> distributionX(-1.5f, 4.0f);
	float randomValueX;

	std::uniform_real_distribution<float> distributionZ(-2.5f, 2.5f);
	float randomValueZ;

	spherePos = new float* [randomRow];
	for (int i = 0; i < randomRow; ++i) {
		spherePos[i] = new float[2]; // 각 행에 2개의 열을 가진 배열 할당
	}

	/*for (int i = 0; i < randomRow; i++)
		spherePos[i] = new float[2];*/

	spherePos[0][0] = -2.7f;
	spherePos[0][1] = 0;

	for (int i = 1; i < randomRow-1; i++) {
		do {
			randomValueX = distributionX(gen);
			randomValueZ = distributionZ(gen);
		} while (isOverlap(randomValueX, randomValueZ, spherePos[i - 1][0], spherePos[i - 1][1]));

		
		spherePos[i][0] = randomValueX;
		spherePos[i][1] = randomValueZ;
	}

	spherePos[randomRow - 1][0] = -2.7f;
	spherePos[randomRow - 1][1] = -0.9f;

	int i = 0;
	g_paddle = new Paddle();
	if (false == g_paddle->create(Device, d3d::WHITE)) return false;
	g_paddle->setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
	g_paddle->setPower(0, 0);
	g_sphere.push_back(g_paddle);

	for (int i = 1; i < randomRow-1; i++) {
		Block* block = new Block(g_point);
		if (false == block->create(Device, d3d::YELLOW)) return false;
		block->setCenter(spherePos[i][0], (float)M_RADIUS	, spherePos[i][1]);
		block->setPower(0, 0);
		g_sphere.push_back(block);
	}

	Bullet* bullet = new Bullet(g_life, *g_paddle);
	if (false == bullet->create(Device, d3d::RED)) return false;
	bullet->setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
	bullet->setPower(0, 0);
	g_sphere.push_back(bullet);
}

// initialization
bool Setup()
{
	D3DXMatrixIdentity(&g_mWorld);
	D3DXMatrixIdentity(&g_mView);
	D3DXMatrixIdentity(&g_mProj);

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f);
	if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, -3.06f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f);
	if (false == g_legowall[3].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f);

	if (!initializeSpheres()) {
		return false;
	}

	if (!InitFont()) {
		return false;
	}

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 0.9f;
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;
	if (false == g_light.create(Device, lit))
		return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(-1.0f, 12.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);

	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
		(float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
	g_legoPlane.destroy();
	for (int i = 0; i < 4; i++) {
		g_legowall[i].destroy();
	}
	destroyAllLegoBlock();
	g_light.destroy();

	if (g_pFont != NULL) {
		g_pFont->Release();
		g_pFont = NULL;
	}

}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta) {
	int i = 0;
	int j = 0;


	if (Device)
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		// update the position of each ball. during update, check whether each ball hit by walls.
		for (auto& s : g_sphere) {
			s->ballUpdate(timeDelta);
		}
		for (i = 0; i < 4; i++) {
			for (auto& s : g_sphere) {
				g_legowall[i].hitBy(*s);
			}
		}

		// check whether any two balls hit together and update the direction of balls
		for (i = 0; i < g_sphere.size(); i++) {
			for (j = 0; j < g_sphere.size(); j++) {
				if (i == j) { continue; }
				g_sphere[i]->hitBy(*g_sphere[j]);
			}
		}

		for (vector<CSphere*>::iterator it = g_sphere.begin(); it != g_sphere.end();) {
			CSphere* s = *it;
			if (s->isRemoving()) {
				it = g_sphere.erase(it);
				delete s;
			}
			else {
				it++;
			}
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i = 0; i < 4; i++) {
			g_legowall[i].draw(Device, g_mWorld);
		}
		for (vector<CSphere*>::iterator it = g_sphere.begin(); it != g_sphere.end(); it++) {
			(*it)->draw(Device, g_mWorld);
		}
		g_light.draw(Device);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture(0, NULL);

		if (g_pFont != NULL) {
			RECT rect_life = { 10, 10, Width, Height };
			std::wstring lifeStr = L"Life: " + std::to_wstring(g_life.getLife());
			g_pFont->DrawTextW(NULL, lifeStr.c_str(), -1, &rect_life, DT_LEFT, d3d::BLACK);

			RECT rect_point = { 10, 10, Width, Height };
			std::wstring pointStr = L"Point: " + std::to_wstring(g_point.getPoint());
			g_pFont->DrawTextW(NULL, lifeStr.c_str(), -1, &rect_point, DT_LEFT, d3d::BLACK);
		}
		//life와 point 화면에 출력

		if (g_life.isDead() || (g_point.getPoint() == g_sphere.size() - 2)) {
			for (vector<CSphere*>::iterator it = g_sphere.begin(); it != g_sphere.end();) {
				CSphere* s = *it;
				it = g_sphere.erase(it);
				delete s;
			}
			g_point = Point();
			g_life = Life();
			initializeSpheres();
		}
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;
	static int old_x = 0;
	static int old_y = 0;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_SPACE:
			Bullet* bullet = dynamic_cast<Bullet*>(g_sphere.back());
			// 스페이스 바를 눌렀을 때 처리
			if (g_sphere.size() > 0 && bullet->getIsSpace()) {
				// 가정: 총알은 g_sphere 벡터의 마지막 원소로 가정
				bullet->shootPressed();
			}
		}

		break;
	}
	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;

		dx = old_x - new_x;
		dy = old_y - new_y;
		if (LOWORD(wParam) && MK_LBUTTON) {
			g_paddle->mouseMoved(g_paddle->getCenter().z + dx * (0.007f));
		}
		old_x = new_x;
		old_y = new_y;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL)));

	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}