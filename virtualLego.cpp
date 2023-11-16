////////////////////////////////////////////////////////////////////////////////
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

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1024;
const int Height = 768;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3)
const float spherePos[4][2] = { {-2.7f,0} , {+2.4f,0} , {3.3f,0} , {-2.7f,-0.9f}}; 
// initialize the color of each ball (ball0 ~ ball3)
const D3DXCOLOR sphereColor[4] = {d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE};

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

class CSphere2 {
protected:
    float center_x, center_y, center_z;
    float m_radius;
    float m_velocity_x;
    float m_velocity_z;
    float distance(CSphere2& ball) {
        float dx = center_x - ball.center_x;
        float dz = center_z - ball.center_z;

        return sqrt(dx * dx + dz * dz);
    }


public:
    CSphere2(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = 0;
        m_velocity_x = 0;
        m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere2(void) {}

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

    bool hasIntersected(CSphere2& ball)
    {
        return distance(ball) < getRadius() + ball.getRadius();
    }

    virtual void hitBy(CSphere2& ball) = 0;

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
    void decrease() {

    }

    int getLife() {
        return 3;
    }

    boolean isDead() {
        return false;
    }
};


class Paddle : public CSphere2 {
public:
    virtual void hitBy(CSphere2& ball) {

    }

    virtual void ballUpdate(float timeDiff) {

    }

    virtual boolean isRemoving() {
        return false;
    }
};

class Bullet : public CSphere2 {
public:
    Bullet(Life& life, Paddle& paddle) {

    }
    virtual void hitBy(CSphere2& ball) {

    }

    virtual void ballUpdate(float timeDiff) {

    }

    virtual boolean isRemoving() {
        return false;
    }
};


class Point {
public:
    void increase() {

    }

    int getPoint() {
        return 0;
    }
};

class Block : public CSphere2 {
public:
    Block(Point& point) {

    }
    virtual void hitBy(CSphere2& ball) {

    }

    virtual void ballUpdate(float timeDiff) {

    }

    virtual boolean isRemoving() {
        return false;
    }
};

class CSphere {
private :
	float					center_x, center_y, center_z;
    float                   m_radius;
	float					m_velocity_x;
	float					m_velocity_z;
    float distance(CSphere& ball)
    {
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
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
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
	
	void hitBy(CSphere& ball) 
	{ 
        if (hasIntersected(ball)) {
            float x_velocity = ball.m_velocity_x - m_velocity_x;
            float z_velocity = ball.m_velocity_z - m_velocity_z;
            float dx = center_x - ball.center_x;
            float dz = center_z - ball.center_z;
            float dot_product = dx * x_velocity + dz * z_velocity;
            if (dot_product > 0) {
                float collision_scale = dot_product / (dx * dx + dz * dz);
                float x_collision = dx * collision_scale;
                float z_collision = dz * collision_scale;

                m_velocity_x += x_collision;
                m_velocity_z += z_collision;
                ball.m_velocity_x -= x_collision;
                ball.m_velocity_z -= z_collision;
            }
        }
	}

	void ballUpdate(float timeDiff) 
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if(vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE*timeDiff*m_velocity_x;
			float tZ = cord.z + TIME_SCALE*timeDiff*m_velocity_z;

			//correction of position of ball
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			/*if(tX >= (4.5 - M_RADIUS))
				tX = 4.5 - M_RADIUS;
			else if(tX <=(-4.5 + M_RADIUS))
				tX = -4.5 + M_RADIUS;
			else if(tZ <= (-3 + M_RADIUS))
				tZ = -3 + M_RADIUS;
			else if(tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;*/
            if (tX >= (4.5 - M_RADIUS)) {
                tX = 4.5 - M_RADIUS;
                m_velocity_x = -m_velocity_x; // 벽에 부딪혔을 때 반사
            }
            else if (tX <= (-4.5 + M_RADIUS)) {
                tX = -4.5 + M_RADIUS;
                m_velocity_x = -m_velocity_x; // 벽에 부딪혔을 때 반사
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
		else { this->setPower(0,0);}
		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 1 -  (1 - DECREASE_RATE)*timeDiff * 400;
		if(rate < 0 )
			rate = 0;
		this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate);
	}

	double getVelocity_X() { return this->m_velocity_x;	}
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x=x;	center_y=y;	center_z=z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
	float getRadius(void)  const { return (float)(M_RADIUS);  }
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
    ID3DXMesh*              m_pSphereMesh;
	
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
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
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

    bool hasIntersected(CSphere2& ball) {
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

    void hitBy(CSphere2& ball) {
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
	
	
	
private :
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	
	D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pBoundMesh;

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
		
        m_lit.Type          = lit.Type;
        m_lit.Diffuse       = lit.Diffuse;
        m_lit.Specular      = lit.Specular;
        m_lit.Ambient       = lit.Ambient;
        m_lit.Position      = lit.Position;
        m_lit.Direction     = lit.Direction;
        m_lit.Range         = lit.Range;
        m_lit.Falloff       = lit.Falloff;
        m_lit.Attenuation0  = lit.Attenuation0;
        m_lit.Attenuation1  = lit.Attenuation1;
        m_lit.Attenuation2  = lit.Attenuation2;
        m_lit.Theta         = lit.Theta;
        m_lit.Phi           = lit.Phi;
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
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
vector<CSphere> g_sphere;
vector<CSphere2*> g_sphere2;
CSphere	g_target_blueball;
CLight	g_light;
Point g_point;
Life g_life;

double g_camera_pos[3] = {0.0, 5.0, -8.0};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;
	
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

	// create four balls and set the position
	for (i=0;i<4;i++) {
        CSphere s;
		if (false == s.create(Device, sphereColor[i])) return false;
		s.setCenter(spherePos[i][0], (float)M_RADIUS , spherePos[i][1]);
		s.setPower(0,0);
        g_sphere.push_back(s);
	}

    Paddle* paddle = new Paddle();
    if (false == paddle->create(Device, d3d::WHITE)) return false;
    paddle->setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
    paddle->setPower(0, 0);
    g_sphere2.push_back(paddle);

    for (i = 1;i < 3;i++) {
        Block* block = new Block(g_point);
        if (false == block->create(Device, d3d::YELLOW)) return false;
        block->setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
        block->setPower(0, 0);
        g_sphere2.push_back(block);
    }

    Bullet* bullet = new Bullet(g_life, *paddle);
    if (false == bullet->create(Device, d3d::RED)) return false;
    bullet->setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
    bullet->setPower(0, 0);
    g_sphere2.push_back(bullet);
	
	// light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type         = D3DLIGHT_POINT;
    lit.Diffuse      = d3d::WHITE; 
	lit.Specular     = d3d::WHITE * 0.9f;
    lit.Ambient      = d3d::WHITE * 0.9f;
    lit.Position     = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range        = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;
	
	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
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
	for(int i = 0 ; i < 4; i++) {
		g_legowall[i].destroy();
	}
    destroyAllLegoBlock();
    g_light.destroy();
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
        for (i = 0; i < 4; i++) {
            g_sphere2[i]->ballUpdate(timeDelta);
            for (j = 0; j < 4; j++) { g_legowall[i].hitBy(*g_sphere2[j]); }
        }

        // check whether any two balls hit together and update the direction of balls
        for (i = 0;i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (i >= j) { continue; }
                g_sphere2[i]->hitBy(*g_sphere2[j]);
            }
        }

        // draw plane, walls, and spheres
        g_legoPlane.draw(Device, g_mWorld);
        for (i = 0;i < 4;i++) {
            g_legowall[i].draw(Device, g_mWorld);
            g_sphere2[i]->draw(Device, g_mWorld);
        }
        g_light.draw(Device);

        Device->EndScene();
        Device->Present(0, 0, 0, 0);
        Device->SetTexture(0, NULL);
    }
    return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;
	
	switch( msg ) {
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
			}
			break;
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
	
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}