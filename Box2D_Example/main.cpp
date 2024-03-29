#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <math.h>
#include <set>
#include <vector>
#include <tuple>
#include "GL/freeglut.h"
#include "Box2D/Box2D.h"

#define num_gnd_vertex 34

// Window screen size
int scr_width = 640;
int scr_height = 640;

// world, bodies
b2World* world;
b2Body* ground;
b2Body* ground_2;
b2Body* ground_3;
b2Body* ground_4;
b2Body* ground_5;
b2Body* box;
b2Body* pin_ball;
b2Body* leftFlipper;
b2Body* rightFlipper;
b2Body* ball_shooter;	// 발사대 설정
b2Body* scissor_1;		// 장애물 - 가위
b2Body* scissor_2;
b2Body* prisma_1;		// 장애물 - prismatic
b2Body* prisma_2;
b2Body* prisma_shoot;
b2Body* tmp_gnd;
b2Body* tmp_gnd2;
b2Body* obs_ball[10];
b2Body* obs_box;
b2Body* obs_box_2;
b2Body* obs_float;
b2Body* water;
b2Body* obs_dist1;
b2Body* obs_dist2;
b2Body* obs_hole;
b2Body* obs_whitehole;
b2Body* obs_large;
b2Body* obs_small;
b2Body* obs_color;
b2Body* obs_color_2;
b2RevoluteJoint* m_leftJoint;	// Flipper 설정
b2RevoluteJoint* m_rightJoint;
b2RevoluteJoint* m_ball_shooter;	// 발사대 설정
b2RevoluteJoint* m_scissor;
b2RevoluteJoint* m_scissor_2;
b2PrismaticJoint* m_prisma;
b2PrismaticJoint* m_shoot;

// shape 설정
b2ChainShape gnd_shape;
b2ChainShape gnd_shape_2;
b2ChainShape gnd_shape_3;
b2ChainShape gnd_shape_4;
b2ChainShape gnd_shape_5;
b2CircleShape ballshape;
b2CircleShape obs_ball_shape;
b2CircleShape obs_hole_shape;
b2CircleShape obs_whitehole_shape;

b2PolygonShape obs_color_shape;
b2PolygonShape obs_color2_shape;
b2PolygonShape obs_float_shape;
b2PolygonShape obs_box_shape;
b2PolygonShape obs_box_shape_2;
b2PolygonShape water_shape;
b2PolygonShape flip_box;
b2PolygonShape shooter_box;
b2PolygonShape scissor_box;
b2PolygonShape prisma_box;
b2PolygonShape prisma_shoot_box;
b2PolygonShape obs_large_shape;
b2PolygonShape obs_small_shape;

// 그 외 여러 설정값들
int32 velocityIterations = 8;	//the number of iterations for computing the impulses
int32 positionIterations = 3;	//the number of iterations for adjusting the position

float32 g_hz = 60.0f;			//frequency
float32 timeStep = 1.0f / g_hz;

// keyboard flag 변수
int flag_flip = false;
int flag_shoot = true;
int flag_color = false;
int flag_ball_color = false;

// gravity 설정
b2Vec2 gravity;

using namespace std;
typedef pair<b2Fixture*, b2Fixture*> fixturePair;
typedef pair<pair<b2Fixture*, b2Fixture*>, pair<float, float>> fixtureTuple;
int m_button = true;

/* contact listener - 일반 장애물 용도 */
class b2ContactListener_ : public b2ContactListener
{
public:
	set<fixturePair> m_fixturePairs;
	set<fixtureTuple> m_fixtureTuples;

	b2ContactListener_() {};

	void BeginContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		b2WorldManifold waterManifold;
		contact->GetWorldManifold(&waterManifold);
		waterManifold.normal.Normalize();

		// 부력
		if (fixtureA->GetBody() == water || fixtureB->GetBody() == water)
		{
			if (fixtureA->IsSensor() && fixtureB->GetBody()->GetType() == b2_dynamicBody)
			{
				m_fixtureTuples.insert(make_pair(make_pair(fixtureB, fixtureA),
					make_pair(waterManifold.normal.x, waterManifold.normal.y)));
			}
			else if (fixtureB->IsSensor() && fixtureA->GetBody()->GetType() == b2_dynamicBody)
			{
				m_fixtureTuples.insert(make_pair(make_pair(fixtureA, fixtureB),
					make_pair(waterManifold.normal.x, waterManifold.normal.y)));
			}
		}
		// 장애물
		else {
			if (fixtureA->IsSensor() && fixtureB->GetBody()->GetType() == b2_dynamicBody) {
				m_fixturePairs.insert(make_pair(fixtureB, fixtureA));
			}
			else if (fixtureB->IsSensor() && fixtureA->GetBody()->GetType() == b2_dynamicBody) {
				m_fixturePairs.insert(make_pair(fixtureA, fixtureB));
			}
		}		
	}

	void EndContact(b2Contact* contact) {
		b2Fixture* fixtureA = contact->GetFixtureA();
		b2Fixture* fixtureB = contact->GetFixtureB();

		b2WorldManifold waterManifold;
		contact->GetWorldManifold(&waterManifold);
		
		waterManifold.normal.Normalize();
		
		// 부력
		if (fixtureA->GetBody() == water || fixtureB->GetBody() == water)
		{
			printf("빠지긴하니?\n");

			if (fixtureA->IsSensor() && fixtureB->GetBody()->GetType() == b2_dynamicBody)
			{
				m_fixtureTuples.erase(make_pair(make_pair(fixtureB, fixtureA),
					make_pair(waterManifold.normal.x, waterManifold.normal.y)));
			}
			else if (fixtureB->IsSensor() && fixtureA->GetBody()->GetType() == b2_dynamicBody)
			{
				m_fixtureTuples.erase(make_pair(make_pair(fixtureA, fixtureB),
					make_pair(waterManifold.normal.x, waterManifold.normal.y)));
			}
		}
		// 장애물
		else {
			if (fixtureA->IsSensor() && fixtureB->GetBody()->GetType() == b2_dynamicBody) {
				m_fixturePairs.erase(make_pair(fixtureB, fixtureA));
			}
			else if (fixtureB->IsSensor() && fixtureA->GetBody()->GetType() == b2_dynamicBody) {
				m_fixturePairs.erase(make_pair(fixtureA, fixtureB));
			}
		}		
	}
};


// contact Listener 설정 - 부력 용도
b2ContactListener_ contactListener;

/* 부력 용도 여러 함수 정리 */
bool inside(b2Vec2 cp1, b2Vec2 cp2, b2Vec2 p) {
	return (cp2.x - cp1.x) * (p.y - cp1.y) > (cp2.y - cp1.y) * (p.x - cp1.x);
}

b2Vec2 ComputeCentroid(vector<b2Vec2> vs, float& area) {
	int count = (int)vs.size();
	b2Assert(count >= 3);

	b2Vec2 c;
	c.Set(0.0f, 0.0f);
	area = 0.0f;

	b2Vec2 pRef(0.0f, 0.0f);
	const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < count; ++i) {
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = vs[i];
		b2Vec2 p3 = i + 1 < count ? vs[i + 1] : vs[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);
		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		c += triangleArea * inv3 * (p1 + p2 + p3);
	}

	if (area > b2_epsilon)
		c *= 1.0f / area;
	else
		area = 0;
	return c;
}

// 부력 적용
void applybuoyancy(b2Fixture* ball, b2Fixture* water, float area,
	b2Vec2 gravity, b2Vec2 centroid)
{
	float displacedMass = water->GetDensity() * area;
	b2Vec2 buoyancyForce = displacedMass * -1.0f * gravity;
	ball->GetBody()->ApplyForce(buoyancyForce, centroid, true);
	printf("buoyancy: %f, %f\n", buoyancyForce.x, buoyancyForce.y);
}

// 드래그 적용
void applydrag(b2Fixture* ball, b2Fixture* water, float area, b2Vec2 centroid)
{
	// 오브젝트와 유체간의 상대속도
	b2Vec2 velDir = ball->GetBody()->GetLinearVelocityFromWorldPoint(centroid)
		- water->GetBody()->GetLinearVelocityFromWorldPoint(centroid);
	float velsquare = velDir.LengthSquared();
	// 속도값 오류 보정
	if (velsquare > 50.0f)
		velsquare = sqrt(velsquare);
	velDir.Normalize();

	// 항력계수
	float cd = 4.0;

	// 간단한 linear drag 적용
	float dragMag = water->GetDensity() * velsquare * area * cd / 2;
	printf("area: %f, vel: %f, dragMag: %f\n", area, velsquare, dragMag);
	b2Vec2 dragForce = dragMag * -velDir;
	ball->GetBody()->ApplyForce(dragForce, centroid, true);

	// 간단한 angular drag 적용
	float angularDrag = cd * area * -water->GetBody()->GetAngularVelocity();
	ball->GetBody()->ApplyTorque(angularDrag, true);
}


b2Vec2 intersection(b2Vec2 cp1, b2Vec2 cp2, b2Vec2 s, b2Vec2 e) {
	b2Vec2 dc(cp1.x - cp2.x, cp1.y - cp2.y);
	b2Vec2 dp(s.x - e.x, s.y - e.y);

	float n1 = cp1.x * cp2.y - cp1.y * cp2.x;
	float n2 = s.x * e.y - s.y * e.x;
	float n3 = 1.0 / (dc.x * dp.y - dc.y * dp.x);

	return b2Vec2((n1 * dp.x - n2 * dc.x) * n3, (n1 * dp.y - n2 * dc.y) * n3);
}

/* 시각화를 위한 함수 */
void Render()
{
	// (1) Initialize glut - 기본 꼴. 수정할 필요 전혀 없음
	if (flag_color == false)
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluOrtho2D(-25.0f, 25.0f, -5.0f, 55.0f);

	/* (2) 맵 그리기 */
	b2Vec2 position = ground->GetPosition();
	float32 angle = ground->GetAngle();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.1f, 0.1f, 0.1f);

	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < num_gnd_vertex; i++)
		glVertex2d(gnd_shape.m_vertices[i].x, gnd_shape.m_vertices[i].y);

	glEnd();
	glPopMatrix();

	// 맵 윗 구멍 만들기
	position = ground_3->GetPosition();
	angle = ground_3->GetAngle();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.1f, 0.1f, 0.1f);

	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 8; i++)
		glVertex2d(gnd_shape_3.m_vertices[i].x, gnd_shape_3.m_vertices[i].y);

	glEnd();
	glPopMatrix();

	// 맵 오른쪽 구멍 만들기
	position = ground_4->GetPosition();
	angle = ground_4->GetAngle();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.1f, 0.1f, 0.1f);

	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 9; i++)
		glVertex2d(gnd_shape_4.m_vertices[i].x, gnd_shape_4.m_vertices[i].y);

	glEnd();
	glPopMatrix();

	// 맵 왼쪽 구멍 만들기
	position = ground_5->GetPosition();
	angle = ground_5->GetAngle();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.1f, 0.1f, 0.1f);

	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 8; i++)
		glVertex2d(gnd_shape_5.m_vertices[i].x, gnd_shape_5.m_vertices[i].y);

	glEnd();
	glPopMatrix();

	// 맵 아랫 구멍 만들기
	position = ground_2->GetPosition();
	angle = ground_2->GetAngle();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.1f, 0.1f, 0.1f);

	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 3; i++)
		glVertex2d(gnd_shape_2.m_vertices[i].x, gnd_shape_2.m_vertices[i].y);

	glEnd();
	glPopMatrix();

	/* (3) 핀볼 그리기 */
	position = pin_ball->GetPosition();
	angle = pin_ball->GetAngle();

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);

	if (flag_ball_color == false)
		glColor3f(0.9f, 0.1f, 0.9f);
	else
		glColor3f(0.9f, 0.9f, 0.9f);


	double rad = ballshape.m_radius;

	glBegin(GL_POLYGON);
	for (int i = 0; i < 360; i++)
	{
		double angle = i * 3.141592 / 180;
		double x = rad * cos(angle);
		double y = rad * sin(angle);
		glVertex2f(x, y);
	}
	glEnd();
	glPopMatrix();

	/* (4) flipper 그리기 - 왼쪽 */
	position = leftFlipper->GetPosition();
	angle = leftFlipper->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.5f, 0.1f, 1.0f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(flip_box.m_vertices[i].x, flip_box.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	/* (5) flipper 그리기 - 오른쪽 */
	position = rightFlipper->GetPosition();
	angle = rightFlipper->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.5f, 0.1f, 1.0f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(flip_box.m_vertices[i].x, flip_box.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	/* (6) scissor 그리기 */
	position = scissor_1->GetPosition();
	angle = scissor_1->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.5f, 0.1f, 1.0f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(scissor_box.m_vertices[i].x, scissor_box.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	position = scissor_2->GetPosition();
	angle = scissor_2->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.5f, 0.1f, 1.0f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(scissor_box.m_vertices[i].x, scissor_box.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();


	/* (7) prismatic 장애물 그리기 */
	position = prisma_1->GetPosition();
	angle = prisma_1->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.5f, 0.9f, 0.4f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(prisma_box.m_vertices[i].x, prisma_box.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	/* (8) 작은 공 장애물 그리기 */
	for (int ball_idx = 0; ball_idx < 10; ball_idx++) {
		glPushMatrix();
		glTranslatef(obs_ball[ball_idx]->GetPosition().x, obs_ball[ball_idx]->GetPosition().y, 0.0f);
		glRotatef(obs_ball[ball_idx]->GetAngle(), 0.0f, 0.0f, 1.0f);
		glColor3f(0.6f, 0.4f, 0.1f);

		double rad = obs_ball_shape.m_radius;

		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++)
		{
			double angle = i * 3.141592 / 180;
			double x = rad * cos(angle);
			double y = rad * sin(angle);
			glVertex2f(x, y);
		}
		glEnd();
		glPopMatrix();
	}

	/* (9) 마름모 부스터 장애물 그리기 */
	position = obs_box->GetPosition();
	angle = obs_box->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.9f, 0.9f, 0.2f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(obs_box_shape.m_vertices[i].x, obs_box_shape.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	position = obs_box_2->GetPosition();
	angle = obs_box_2->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.9f, 0.9f, 0.2f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(obs_box_shape_2.m_vertices[i].x, obs_box_shape_2.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	/* (10) 부력 */
	position = water->GetPosition();
	angle = water->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.5f, 0.5f, 0.9f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(water_shape.m_vertices[i].x, water_shape.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();


	/* (12) 발사대 그리기 */
	position = prisma_shoot->GetPosition();
	angle = prisma_shoot->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.9f, 0.2f, 0.2f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(prisma_shoot_box.m_vertices[i].x, prisma_shoot_box.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	/* (13) 블랙홀 그리기 */
	position = obs_hole->GetPosition();
	angle = obs_hole->GetAngle();

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.9f, 0.5f, 0.2f);

	double hole_rad = obs_hole_shape.m_radius;

	glBegin(GL_POLYGON);
	for (int i = 0; i < 360; i++)
	{
		double angle = i * 3.141592 / 180;
		double x = hole_rad * cos(angle);
		double y = hole_rad * sin(angle);
		glVertex2f(x, y);
	}
	glEnd();
	glPopMatrix();

	/* (14) 화이트홀 그리기 */
	float white_x = 3.0f, white_y = 5.0f;

	glPushMatrix();
	glTranslatef(white_x, white_y, 0.0f);
	glRotatef(0, 0.0f, 0.0f, 1.0f);
	glColor3f(0.8f, 0.8f, 0.8f);

	double whitehole_rad = 1.0f;

	glBegin(GL_POLYGON);
	for (int i = 0; i < 360; i++)
	{
		double angle = i * 3.141592 / 180;
		double x = whitehole_rad * cos(angle);
		double y = whitehole_rad * sin(angle);
		glVertex2f(x, y);
	}
	glEnd();
	glPopMatrix();

	/* (15) 부력 장애물 */
	position = obs_float->GetPosition();
	angle = obs_float->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.9f, 0.2f, 0.2f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(obs_float_shape.m_vertices[i].x, obs_float_shape.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	/* (16) 확대 축소 장애물 그리기 */
	position = obs_large->GetPosition();
	angle = obs_large->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.1f, 0.1f, 0.9f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(obs_large_shape.m_vertices[i].x, obs_large_shape.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	position = obs_small->GetPosition();
	angle = obs_small->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.1f, 0.9f, 0.9f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(obs_small_shape.m_vertices[i].x, obs_small_shape.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	/* (17) 색깔 변경 장애물 그리기 */
	position = obs_color->GetPosition();
	angle = obs_color->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.9f, 0.4f, 0.4f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(obs_color_shape.m_vertices[i].x, obs_color_shape.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();

	position = obs_color_2->GetPosition();
	angle = obs_color_2->GetAngle() * 180 / b2_pi;

	glPushMatrix();
	glTranslatef(position.x, position.y, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glColor3f(0.5f, 0.5f, 0.9f);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glVertex2f(obs_color2_shape.m_vertices[i].x, obs_color2_shape.m_vertices[i].y);
	}
	glEnd();
	glPopMatrix();


	/* 최종 openGL 코드 */
	glutSwapBuffers();
}

void Update(int value)
{
	// update the simulation
	world->Step(timeStep, velocityIterations, positionIterations);

	// contact listener - 부력 관련 부분
	if (contactListener.m_fixturePairs.size() > 0) {
		set<fixturePair>::iterator it = contactListener.m_fixturePairs.begin();
		set<fixturePair>::iterator end = contactListener.m_fixturePairs.end();

		while (it != end) {
			b2Fixture* fixture_one = it->first;
			b2Fixture* fixture_two = it->second;

			float density = fixture_two->GetDensity();

			vector<b2Vec2> intersectionPoints;

			// 블랙홀인지 확인하기 위함
			if ((fixture_one->GetBody() == obs_hole) || (fixture_two->GetBody() == obs_hole)) {
				b2Vec2 white_pos(5.0f, 10.0f);
				pin_ball->SetTransform(white_pos, pin_ball->GetAngle());
				pin_ball->ApplyForce(b2Vec2(0.0f, -5000.0f), pin_ball->GetWorldCenter(), true);
			}

			// 반사 장애물인지 확인하기 위함
			if ((fixture_one->GetBody() == obs_box) || (fixture_two->GetBody() == obs_box)) {
				pin_ball->ApplyForce(b2Vec2(100.0f, -500.0f), pin_ball->GetWorldCenter(), true);
			}
			if ((fixture_one->GetBody() == obs_box_2) || (fixture_two->GetBody() == obs_box_2)) {
				pin_ball->ApplyForce(b2Vec2(-100.0f, -500.0f), pin_ball->GetWorldCenter(), true);
			}

			// 확대 장애물인지 확인하기 위함
			if ((fixture_one->GetBody() == obs_large) || (fixture_two->GetBody() == obs_large)) {
				ballshape.m_radius = 1.0f;
			}

			// 축소 장애물인지 확인하기 위함
			if ((fixture_one->GetBody() == obs_small) || (fixture_two->GetBody() == obs_small)) {
				ballshape.m_radius = 0.5f;
			}

			// 색깔 변경하기 - 흰색
			if ((fixture_one->GetBody() == obs_color) || (fixture_two->GetBody() == obs_color)) {
				glClearColor(0.0f, 0.5f, 0.5f, 0.8f);
				flag_ball_color = true;
				flag_color = true;
			}

			// 색깔 변경하기 - 핑크색
			if ((fixture_one->GetBody() == obs_color_2) || (fixture_two->GetBody() == obs_color_2)) {
				glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
				flag_ball_color = false;
			}
			++it;
		}
	}

	// 부력 관련 contact listner
	if (contactListener.m_fixtureTuples.size() > 0)
	{
		set<fixtureTuple>::iterator it = contactListener.m_fixtureTuples.begin();
		set<fixtureTuple>::iterator end = contactListener.m_fixtureTuples.end();

		while (it != end) {			
			b2Fixture* fixture_A = it->first.first;
			b2Fixture* fixture_B = it->first.second;

			// 부력 적용하는 경우에 해당하는지
			if (fixture_B->GetBody() == water)
			{
				// 접촉점 노멀벡터
				b2Vec2 Mani_BallnWater;
				Mani_BallnWater.x = it->second.first;
				Mani_BallnWater.y = it->second.second;

				// raycast input
				b2RayCastInput input;
				float rayLength = ballshape.m_radius; // 기준을 공 반지름만큼

				input.p1 = pin_ball->GetPosition(); // 공의 중심 좌표

				// 공이 절반보다 덜 잠겨있는지 확인
				input.p2 = input.p1 + rayLength * (-1.0f) * Mani_BallnWater;
				input.maxFraction = 20.0f; // 충분히 닿을만큼

				b2RayCastOutput output;

				// ray 교차점
				b2Vec2 intersectionPoint;
				// 면적, 부력 작용점 계산

				float area = 0.0f;
				b2Vec2 centroid;

				// 부딪힐 경우
				if (fixture_B->RayCast(&output, input, 0))
				{
					printf("Here  ");
					float origin_area = pow(ballshape.m_radius, 2) * b2_pi; // 원 전체 넓이

					if (output.fraction <= 1.0f)  // 절반 이하로 잠겼을 경우
					{
						printf("A  ");
						// ray 교점
						intersectionPoint = input.p1 + output.fraction * (input.p2 - input.p1);
						float dist = b2Distance(intersectionPoint, input.p1);
						double cos = dist / ballshape.m_radius; // cos(세타/2)
						double angle = 2.0f * acos(cos); // 라디안값
						// 넓이 = 부채꼴 - 삼각형
						area = (pow(ballshape.m_radius, 2) * angle * 0.5f) - (pow(ballshape.m_radius, 2) * sin(angle) * 0.5f);
						// 작용점 계산
						centroid = input.p1 + 4.0f * pow(sin(angle * 0.5f), 3) / (3.0f * (angle - sin(angle))) * (input.p2 - input.p1);

					}
				}
				// 안에 들어와있는지 점검
				else
				{
					printf("여긴가  ");
					// 안 잠겼거나, 절반 이상 잠겼을 경우
					input.p1 = input.p1 + rayLength * 1.0f * Mani_BallnWater; // 노멀 따라서 원위의 점으로
					input.p2 = pin_ball->GetPosition();
					if (fixture_B->RayCast(&output, input, 0))
					{
						printf("B  ");
						//절반 보다는 더 잠겼으나, 완전히 덜 잠겼을 경우
						float new_fraction = 1 - output.fraction;
						if (new_fraction <= 1.0f)
						{
							// ray 교점
							intersectionPoint = input.p1 + output.fraction * (input.p2 - input.p1);
							float dist = b2Distance(intersectionPoint, input.p1);
							double cos = dist / ballshape.m_radius; // cos(세타/2)
							double angle = 2.0f * acos(cos); // 라디안값
							// 넓이 = 부채꼴 + 삼각형
							area = (pow(ballshape.m_radius, 2) * (2.0f * b2_pi - angle) * 0.5f) + (pow(ballshape.m_radius, 2) * sin(angle) * 0.5f);
							// 작용점 계산
							centroid = input.p1 + 4.0f * pow(sin(angle * 0.5f), 3) / (3.0f * (angle - sin(angle))) * (input.p1 - input.p2);

						}
					}
					// 완전히 잠겼을 경우
					else
					{
						printf("C  ");
						area = pow(ballshape.m_radius, 2) * b2_pi;
						centroid = pin_ball->GetPosition();
					}
				}

				printf("area: %f\n", area);

				// 부력적용
				applybuoyancy(fixture_A, fixture_B, area, gravity, centroid);

				// drag force 적용
				applydrag(fixture_A, fixture_B, area, centroid);
			}
			++it;
		}
	}


	// 키보드 컨트롤 부분 - flipper
	if (flag_flip == true) {
		m_leftJoint->SetMotorSpeed(40.0f);
		m_rightJoint->SetMotorSpeed(-40.0f);
	}
	else {
		m_leftJoint->SetMotorSpeed(-40.0f);
		m_rightJoint->SetMotorSpeed(40.0f);
	}

	// 키보드 컨트롤 부분 - 발사대
	if (flag_shoot == true) {
		m_shoot->SetMotorSpeed(1000.0f);
	}
	else {
		m_shoot->SetMotorSpeed(-10.0f);
	}

	// 왔다갔다 장애물 처리
	const float max_speed = 5.0f;
	if (m_prisma->GetJointTranslation() > 4.0f) {
		m_prisma->SetMotorSpeed(-10.0f);
	}
	else if (m_prisma->GetJointTranslation() < -4.0f) {
		m_prisma->SetMotorSpeed(10.0f);
	}

	// 마지막 부분
	glutPostRedisplay();
	glutTimerFunc(20, Update, 0);	//Recursive function
}

void Reshape(int _width, int _height)
{
	scr_width = _width;
	scr_height = _height;
	glViewport(0, 0, _width, _height);
}

/* body, fixture 등 설정하는 부분 */
void Setup()
{
	// 중력 설정	
	gravity.Set(0.0f, -10.0f);

	// world 설정
	world = new b2World(gravity);
	world->SetContactListener(&contactListener);
	//world->SetContactListener(&water_contactListener);

	// 맵 설정
	{
		// step1 : define body - 위치 설정
		b2BodyDef gnd_bd;

		// step2 : create body
		b2Body* _ground = world->CreateBody(&gnd_bd);

		// step3 : create shape		
		b2Vec2 vs[num_gnd_vertex];
		int idx = 0;

		// 발사대 부분
		vs[idx++].Set(-22.0f, -4.0f);
		vs[idx++].Set(-22.0f, 47.0f);
		vs[idx++].Set(-21.0f, 50.0f);

		// 몸통 부분
		vs[idx++].Set(-7.0f, 33.0f);
		vs[idx++].Set(-5.0f, 35.0f);
		vs[idx++].Set(-2.0f, 37.0f);
		vs[idx++].Set(1.0f, 38.0f);
		vs[idx++].Set(5.0f, 38.0f);
		vs[idx++].Set(7.0f, 37.0f);
		vs[idx++].Set(10.0f, 35.0f);
		vs[idx++].Set(15.0f, 30.0f);
		vs[idx++].Set(16.0f, 28.0f);//new
		vs[idx++].Set(17.0f, 25.0f);
		vs[idx++].Set(17.0f, 23.0f);
		vs[idx++].Set(17.0f, 20.0f);//new end
		vs[idx++].Set(15.0f, 15.0f);

		// 수영장 부분
		vs[idx++].Set(5.0f, 2.0f);	// flipper 필요
		vs[idx++].Set(5.0f, 0.0f);
		vs[idx++].Set(10.0f, 0.0f);
		vs[idx++].Set(15.0f, 0.0f);
		vs[idx++].Set(15.0f, -4.0f);
		vs[idx++].Set(-10.0f, -4.0f);
		vs[idx++].Set(-10.0f, 0.0f);
		vs[idx++].Set(1.0f, 0.0f);
		vs[idx++].Set(1.0f, 2.0f);	// flipper 필요

		// 몸통 부분
		vs[idx++].Set(-10.0f, 15.0f);
		vs[idx++].Set(-12.0f, 20.0f);	// new start
		vs[idx++].Set(-12.0f, 25.0f);	// new end
		vs[idx++].Set(-10.0f, 30.0f);
		vs[idx++].Set(-8.0f, 32.0f);

		// 발사대 부분
		vs[idx++].Set(-18.0f, 43.0f);
		vs[idx++].Set(-20.0f, 42.5f);
		vs[idx++].Set(-20.0f, -4.0f);
		vs[idx++].Set(-22.0f, -4.0f);
		gnd_shape.CreateLoop(vs, num_gnd_vertex);

		// step4 : create fixture - 기타 설정
		b2FixtureDef fd;
		fd.shape = &gnd_shape;
		fd.density = 0.5f;

		// step5 : attach shape to body with fixture
		_ground->CreateFixture(&fd);
		ground = _ground;
	}

	// 맵 윗구멍 설정
	{
		// step1 : define body - 위치 설정
		b2BodyDef gnd_bd;

		// step2 : create body
		b2Body* _ground = world->CreateBody(&gnd_bd);

		// step3 : create shape				
		b2Vec2 vs[8];
		int idx = 0;

		vs[idx++].Set(-3.0f, 33.0f);
		vs[idx++].Set(-1.0f, 35.0f);
		vs[idx++].Set(1.5f, 36.0f);
		vs[idx++].Set(4.0f, 36.0f);
		vs[idx++].Set(6.0f, 35.0f);
		vs[idx++].Set(8.0f, 33.0f);
		vs[idx++].Set(5.0f, 30.0f);
		vs[idx++].Set(0.0f, 30.0f);

		gnd_shape_3.CreateLoop(vs, idx);

		// step4 : create fixture - 기타 설정
		b2FixtureDef fd;
		fd.shape = &gnd_shape_3;
		fd.density = 0.5f;

		// step5 : attach shape to body with fixture
		_ground->CreateFixture(&fd);
		ground_3 = _ground;
	}

	// 맵 오른쪽 구멍 설정
	{
		// step1 : define body - 위치 설정
		b2BodyDef gnd_bd;

		// step2 : create body
		b2Body* _ground = world->CreateBody(&gnd_bd);

		// step3 : create shape				
		b2Vec2 vs[9];
		int idx = 0;

		vs[idx++].Set(10.0f, 25.0f);
		vs[idx++].Set(13.0f, 28.0f);
		vs[idx++].Set(14.5f, 26.0f);
		vs[idx++].Set(15.0f, 25.0f);
		vs[idx++].Set(15.0f, 22.0f);
		vs[idx++].Set(14.5f, 19.0f);
		vs[idx++].Set(14.0f, 18.0f);
		vs[idx++].Set(13.0f, 17.0f);
		vs[idx++].Set(10.0f, 20.0f);

		gnd_shape_4.CreateLoop(vs, idx);

		// step4 : create fixture - 기타 설정
		b2FixtureDef fd;
		fd.shape = &gnd_shape_4;
		fd.density = 0.5f;

		// step5 : attach shape to body with fixture
		_ground->CreateFixture(&fd);
		ground_4 = _ground;
	}

	// 맵 왼쪽 구멍 설정
	{
		// step1 : define body - 위치 설정
		b2BodyDef gnd_bd;

		// step2 : create body
		b2Body* _ground = world->CreateBody(&gnd_bd);

		// step3 : create shape				
		b2Vec2 vs[8];
		int idx = 0;

		vs[idx++].Set(-5.0f, 25.0f);
		vs[idx++].Set(-5.0f, 20.0f);
		vs[idx++].Set(-6.0f, 19.0f);
		vs[idx++].Set(-8.0f, 20.0f);
		vs[idx++].Set(-9.0f, 22.0f);
		vs[idx++].Set(-9.0f, 25.0f);
		vs[idx++].Set(-8.5f, 27.0f);
		vs[idx++].Set(-8.0f, 28.0f);

		gnd_shape_5.CreateLoop(vs, idx);

		// step4 : create fixture - 기타 설정
		b2FixtureDef fd;
		fd.shape = &gnd_shape_5;
		fd.density = 0.5f;

		// step5 : attach shape to body with fixture
		_ground->CreateFixture(&fd);
		ground_5 = _ground;
	}


	// 맵 아래 구멍 설정
	{
		// step1 : define body - 위치 설정
		b2BodyDef gnd_bd;

		// step2 : create body
		b2Body* _ground = world->CreateBody(&gnd_bd);

		// step3 : create shape		
		b2Vec2 vs[3];
		int idx = 0;

		vs[idx++].Set(-3.0f, 15.0f);
		vs[idx++].Set(9.0f, 15.0f);
		vs[idx++].Set(3.0f, 8.0f);

		gnd_shape_2.CreateLoop(vs, 3);

		// step4 : create fixture - 기타 설정
		b2FixtureDef fd;
		fd.shape = &gnd_shape_2;
		fd.density = 0.5f;

		// step5 : attach shape to body with fixture
		_ground->CreateFixture(&fd);
		ground_2 = _ground;
	}

	// Flipper 설정
	{
		/* 기본적인 설정 부분 */
		// step1 : define body - 위치 설정
		b2Vec2 p1(0.5f, 2.0f), p2(5.5f, 2.0f);

		// step2 : create body - 각 flipper별로 만들기
		b2BodyDef bd;
		bd.type = b2_dynamicBody;

		bd.position = p1;
		leftFlipper = world->CreateBody(&bd);

		bd.position = p2;
		rightFlipper = world->CreateBody(&bd);

		// step3 : create shape	
		flip_box.SetAsBox(1.75f, 0.1f);

		// step4 : create fixture - 기타 설정
		b2FixtureDef fd;
		fd.shape = &flip_box;
		fd.density = 1.0f;

		// Step5 : Attach shape to body with fixture
		leftFlipper->CreateFixture(&fd);
		rightFlipper->CreateFixture(&fd);

		/* flipper를 ground에 고정시키기 위한 과정*/
		// body A - the underneath object (=ground)
		b2RevoluteJointDef jd;
		jd.bodyA = ground;
		jd.localAnchorB.SetZero();
		jd.enableMotor = true;
		jd.maxMotorTorque = 1000.0f;
		jd.enableLimit = true;

		// body B - the overhead object (=flipper)
		jd.motorSpeed = -30.0f;
		jd.localAnchorA = p1;
		jd.bodyB = leftFlipper;
		jd.lowerAngle = -30.0f * b2_pi / 180.0f;
		jd.upperAngle = 5.0f * b2_pi / 180.0f;
		m_leftJoint = (b2RevoluteJoint*)world->CreateJoint(&jd);

		jd.motorSpeed = 30.0f;
		jd.localAnchorA = p2;
		jd.bodyB = rightFlipper;
		jd.lowerAngle = -5.0f * b2_pi / 180.0f;
		jd.upperAngle = 30.0f * b2_pi / 180.0f;
		m_rightJoint = (b2RevoluteJoint*)world->CreateJoint(&jd);
	}


	// 배경 검은색 장애물
	{
		// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
		b2BodyDef box;
		box.type = b2_staticBody;

		b2Vec2 p1(-9.0f, 17.5f);
		box.position = p1;

		// Step2 : create body
		b2Body* body = world->CreateBody(&box);

		// Step3 : crate shape - 길이
		obs_color_shape.SetAsBox(1.0f, 1.0f);

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef obs_boxfd;
		obs_boxfd.shape = &obs_color_shape;
		obs_boxfd.density = 1.0f;		// 밀도		
		obs_boxfd.isSensor = true;

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&obs_boxfd);
		obs_color = body;
	}

	// 배경 흰색 장애물
	{
		// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
		b2BodyDef box;
		box.type = b2_staticBody;

		b2Vec2 p1(-4.0f, 17.5f);
		box.position = p1;

		// Step2 : create body
		b2Body* body = world->CreateBody(&box);

		// Step3 : crate shape - 길이
		obs_color2_shape.SetAsBox(1.0f, 1.0f);

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef obs_boxfd;
		obs_boxfd.shape = &obs_color2_shape;
		obs_boxfd.density = 1.0f;		// 밀도		
		obs_boxfd.isSensor = true;

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&obs_boxfd);
		obs_color_2 = body;
	}


	// 바람개비 장애물
	{
		/* 기본적인 설정 부분 */
		// step1 : define body - 위치 설정
		b2Vec2 p1(-6.0f, 30.0f), p2(-3.0f, 30.0f);

		// step2 : create body - 각 scissor별로 만들기
		b2BodyDef bd;
		bd.type = b2_dynamicBody;

		bd.position = p1;
		scissor_1 = world->CreateBody(&bd);

		bd.position = p2;
		scissor_2 = world->CreateBody(&bd);

		// step3 : create shape	
		scissor_box.SetAsBox(1.75f, 0.1f);

		// step4 : create fixture - 기타 설정
		b2FixtureDef fd;
		fd.shape = &scissor_box;
		fd.density = 1.0f;

		b2FixtureDef fd2;
		fd2.shape = &scissor_box;
		fd2.density = 1.0f;

		// Step5 : Attach shape to body with fixture
		scissor_1->CreateFixture(&fd);
		scissor_2->CreateFixture(&fd2);

		/* flipper를 ground에 고정시키기 위한 과정*/
		// body A - the underneath object (=ground)
		b2RevoluteJointDef jd;
		jd.bodyA = ground;
		jd.localAnchorB.SetZero();
		jd.enableMotor = true;
		jd.maxMotorTorque = 1000.0f;

		// body B - the overhead object (=scissor)
		jd.motorSpeed = 30.0f;
		jd.localAnchorA = p1;
		jd.bodyB = scissor_1;

		m_scissor = (b2RevoluteJoint*)world->CreateJoint(&jd);

		// body A - the underneath object (=ground)
		b2RevoluteJointDef jd2;
		jd2.bodyA = ground;
		jd2.localAnchorB.SetZero();
		jd2.enableMotor = true;
		jd2.maxMotorTorque = 1000.0f;

		// body B - the overhead object (=scissor)
		jd2.motorSpeed = -30.0f;
		jd2.localAnchorA = p2;
		jd2.bodyB = scissor_2;

		m_scissor_2 = (b2RevoluteJoint*)world->CreateJoint(&jd2);
	}

	// 핀볼
	{
		// Step1 : define body - 위치
		b2BodyDef ball;
		ball.type = b2_dynamicBody;
		ball.position.Set(-21.0f, -2.0f);

		// Step2 : create body
		b2Body* body = world->CreateBody(&ball);

		// Step3 : crate shape - 길이
		ballshape.m_radius = 0.5;

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef ballfd;
		ballfd.shape = &ballshape;
		ballfd.friction = 0.2f;		// 마찰력
		ballfd.density = 1.0f;		// 밀도
		ballfd.restitution = 0.5f;	// 반발력

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&ballfd);
		pin_ball = body;
	}

	// 작은 공 장애물
	{
		float ball_offset_y = 18.0f, ball_offset_x = 3.0f;

		float tmp_x[10] = { ball_offset_x - 1.0f, ball_offset_x + 0.0f,
			ball_offset_x + 1.0f, ball_offset_x - 0.5f,
			ball_offset_x + 0.5f, ball_offset_x - 0.5f,
			ball_offset_x + 0.5f , ball_offset_x - 1.0f,
			ball_offset_x + 0.0f, ball_offset_x + 1.0f };
		float tmp_y[10] = { ball_offset_y + 9.0f, ball_offset_y + 9.0f ,
			ball_offset_y + 9.0f , ball_offset_y + 8.0f ,
			ball_offset_y + 8.0f , ball_offset_y + 7.0f ,
			ball_offset_y + 7.0f , ball_offset_y + 6.0f ,
			ball_offset_y + 6.0f, ball_offset_y + 6.0f };

		for (int i = 0; i < 10; i++) {
			// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
			b2BodyDef ball;
			ball.type = b2_staticBody;
			ball.position.Set(tmp_x[i], tmp_y[i]);

			// Step2 : create body
			b2Body* body = world->CreateBody(&ball);

			// Step3 : crate shape - 길이
			obs_ball_shape.m_radius = 0.3f;

			// Step4 : create Fixture - 기타 속성
			b2FixtureDef ballfd;
			ballfd.shape = &obs_ball_shape;
			ballfd.friction = 0.2f;		// 마찰력
			ballfd.density = 1.0f;		// 밀도
			ballfd.restitution = 0.5f;	// 반발력

			// Step5 : Attach shape to body with fixture
			body->CreateFixture(&ballfd);
			obs_ball[i] = body;
		}
	}

	// 확대 장애물
	{
		// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
		b2BodyDef box;
		box.type = b2_staticBody;

		b2Vec2 p1(-5.0f, 13.0f);
		box.position = p1;

		// Step2 : create body
		b2Body* body = world->CreateBody(&box);

		// Step3 : crate shape - 길이
		obs_large_shape.SetAsBox(1.0f, 1.0f);

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef obs_boxfd;
		obs_boxfd.shape = &obs_large_shape;
		obs_boxfd.density = 1.0f;		// 밀도		
		obs_boxfd.isSensor = true;

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&obs_boxfd);
		obs_large = body;
	}

	// 축소 장애물
	{
		// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
		b2BodyDef box;
		box.type = b2_staticBody;

		b2Vec2 p1(11.0f, 13.0f);
		box.position = p1;

		// Step2 : create body
		b2Body* body = world->CreateBody(&box);

		// Step3 : crate shape - 길이
		obs_small_shape.SetAsBox(1.0f, 1.0f);

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef obs_boxfd;
		obs_boxfd.shape = &obs_small_shape;
		obs_boxfd.density = 1.0f;		// 밀도		
		obs_boxfd.isSensor = true;

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&obs_boxfd);
		obs_small = body;
	}
	// 마름모 장애물
	{
		// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
		b2BodyDef box;
		box.type = b2_staticBody;

		b2Vec2 p1(-1.0f, 8.0f);
		box.position = p1;

		// Step2 : create body
		b2Body* body = world->CreateBody(&box);

		// Step3 : crate shape - 길이
		obs_box_shape.SetAsBox(1.0f, 1.0f);

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef obs_boxfd;
		obs_boxfd.shape = &obs_box_shape;
		obs_boxfd.density = 1.0f;		// 밀도		
		obs_boxfd.isSensor = true;

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&obs_boxfd);
		obs_box = body;

		/* 두 번째 장애물 */
		// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
		b2Vec2 p2(7.0f, 8.0f);
		box.position = p2;

		// Step2 : create body
		body = world->CreateBody(&box);

		// Step3 : crate shape - 길이
		obs_box_shape_2.SetAsBox(1.0f, 1.0f);

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef obs_boxfd_2;
		obs_boxfd_2.shape = &obs_box_shape_2;
		obs_boxfd_2.density = 1.0f;		// 밀도		
		obs_boxfd_2.isSensor = true;

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&obs_boxfd_2);
		obs_box_2 = body;
	}

	// 부력 장애물
	{
		// Step1 : define body - 위치 설정 따로 안함 (Render에서 설정하기)
		b2BodyDef box;
		box.type = b2_dynamicBody;

		b2Vec2 p1(2.0f, 12.0f);
		box.position = p1;

		// Step2 : create body
		b2Body* body = world->CreateBody(&box);

		// Step3 : crate shape - 길이
		obs_float_shape.SetAsBox(1.0f, 1.0f);

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef obs_float_boxfd;
		obs_float_boxfd.shape = &obs_float_shape;
		obs_float_boxfd.density = 1.0f;		// 밀도		

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&obs_float_boxfd);
		obs_float = body;
	}

	// 와리가리 - prismatic 이용 장애물 
	{
		/* 기본적인 설정 부분 */
		// step1 : define body - 위치 설정
		// step2 : create body 
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(2.0f, 20.0f);
		bd.angle = 0.5f * b2_pi;
		bd.allowSleep = false;
		prisma_1 = world->CreateBody(&bd);

		// step3 : create shape			
		prisma_box.SetAsBox(1.0f, 1.0f);

		// step4 : create fixture - 기타 설정		
		b2FixtureDef fd;
		fd.shape = &prisma_box;
		fd.density = 1.0f;

		// Step5 : Attach shape to body with fixture
		prisma_1->CreateFixture(&fd);

		/* flipper를 ground에 고정시키기 위한 과정*/
		b2PrismaticJointDef pjd;

		// Horizontal
		pjd.Initialize(ground, prisma_1, bd.position, b2Vec2(1.0f, 0.0f));

		pjd.motorSpeed = 10.0f;
		pjd.maxMotorForce = 10000.0f;
		pjd.enableMotor = true;
		pjd.lowerTranslation = -5.0f;
		pjd.upperTranslation = 5.0f;
		pjd.enableLimit = true;

		m_prisma = (b2PrismaticJoint*)world->CreateJoint(&pjd);
	}

	// 핀볼 발사대 용도
	{
		/* 기본적인 설정 부분 */
		// step1 : define body - 위치 설정
		// step2 : create body 
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(-21.0f, -4.0f);
		bd.angle = 0.5f * b2_pi;
		bd.allowSleep = false;
		prisma_shoot = world->CreateBody(&bd);

		// step3 : create shape			
		prisma_shoot_box.SetAsBox(0.1f, 0.9f);

		// step4 : create fixture - 기타 설정		
		b2FixtureDef fd;
		fd.shape = &prisma_shoot_box;
		fd.density = 1.0f;

		// Step5 : Attach shape to body with fixture
		prisma_shoot->CreateFixture(&fd);

		/* flipper를 ground에 고정시키기 위한 과정*/
		b2PrismaticJointDef pjd;

		// Horizontal
		pjd.Initialize(ground, prisma_shoot, bd.position, b2Vec2(0.0f, 1.0f));

		pjd.motorSpeed = 100.0f;
		pjd.maxMotorForce = 10000.0f;
		pjd.enableMotor = true;
		pjd.lowerTranslation = 0.6f;
		pjd.upperTranslation = 5.0f;
		pjd.enableLimit = true;

		m_shoot = (b2PrismaticJoint*)world->CreateJoint(&pjd);
	}

	// 물통 채우기
	{
		// step 1
		b2BodyDef boxbd_water;
		boxbd_water.type = b2_staticBody;
		boxbd_water.position.Set(2.4f, -3.0f);

		// step2
		b2Body* body_water = world->CreateBody(&boxbd_water);

		// step3
		water_shape.SetAsBox(12.4f, 1.0f);

		// step4
		b2FixtureDef boxfd_water;
		boxfd_water.shape = &water_shape;
		boxfd_water.density = 4.0f;
		boxfd_water.restitution = 0.0f;
		boxfd_water.friction = 5.0f;
		boxfd_water.isSensor = true;

		// step5
		body_water->CreateFixture(&boxfd_water);
		water = body_water;
	}

	// 블랙홀
	{
		// Step1 : define body - 위치
		b2BodyDef ball;
		ball.type = b2_staticBody;
		ball.position.Set(9.0f, 29.0f);

		// Step2 : create body
		b2Body* body = world->CreateBody(&ball);

		// Step3 : crate shape - 길이
		obs_hole_shape.m_radius = 1.5;

		// Step4 : create Fixture - 기타 속성
		b2FixtureDef ballfd;
		ballfd.shape = &obs_hole_shape;
		ballfd.friction = 0.2f;		// 마찰력
		ballfd.density = 1.0f;		// 밀도
		ballfd.restitution = 0.5f;	// 반발력
		ballfd.isSensor = true;		// 센서 발동

		// Step5 : Attach shape to body with fixture
		body->CreateFixture(&ballfd);
		obs_hole = body;
	}
}

/* 키보드 입력 그만 했을 때 호출 */
void DoReleaseKey(unsigned char key, int x, int y) {
	int x_force = 20000;
	int y_force = 20000;

	switch (key)
	{
	case 'q':
		pin_ball->ApplyForce(b2Vec2(-20000, 20000), pin_ball->GetWorldCenter(), true);
		break;						 		
	case 'e':						 		
		pin_ball->ApplyForce(b2Vec2(20000, -20000), pin_ball->GetWorldCenter(), true);
		break;
	case 'w':
		pin_ball->ApplyForce(b2Vec2(0, 100), pin_ball->GetWorldCenter(), true);
		break;
	case 'a':
		flag_flip = false;
		break;
	case 'b':
		flag_shoot = false;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

/* 키보드 입력 했을때 호출 */
void DokeyBoard(unsigned char key, int x, int y)
{
	int x_force = 20000;
	int y_force = 20000;

	switch (key)
	{
	case 'q':
		pin_ball->ApplyForce(b2Vec2(-x_force, y_force), pin_ball->GetWorldCenter(), true);
		break;
	case 'e':
		pin_ball->ApplyForce(b2Vec2(x_force, -y_force), pin_ball->GetWorldCenter(), true);
		break;
	case 'a':
		flag_flip = true;
		break;
	case 'b':
		flag_shoot = true;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	// Initialize glut
	glutInitWindowSize(scr_width, scr_height);
	glutInit(&argc, argv);
	glutCreateWindow("Box2D");

	// Setting Box2D elements
	Setup();

	glutDisplayFunc(Render);		//If you want to render, Use it.
	glutReshapeFunc(Reshape);		//Reshape by window size
	glutTimerFunc(20, Update, 0);	//Update physics simulation

	glutKeyboardFunc(DokeyBoard);
	glutKeyboardUpFunc(DoReleaseKey);

	glutMainLoop();

	return 0;
}


//bool findIntersectionOfFixtures(b2Fixture* fA, b2Fixture* fB, vector<b2Vec2>& outputVertices) {
//	if (fA->GetShape()->GetType() != b2Shape::e_polygon ||
//		fB->GetShape()->GetType() != b2Shape::e_polygon)
//		return false;
//
//	b2PolygonShape* polyA = (b2PolygonShape*)fA->GetShape();
//	b2PolygonShape* polyB = (b2PolygonShape*)fB->GetShape();
//
//	for (int i = 0; i < polyA->GetVertexCount(); i++)
//		outputVertices.push_back(fA->GetBody()->GetWorldPoint(polyA->GetVertex(i)));
//
//	vector<b2Vec2> clipPolygon;
//	for (int i = 0; i < polyA->GetVertexCount(); i++)
//		clipPolygon.push_back(fB->GetBody()->GetWorldPoint(polyB->GetVertex(i)));
//
//	b2Vec2 cp1 = clipPolygon[clipPolygon.size() - 1];
//	for (int j = 0; j < clipPolygon.size(); j++) {
//		b2Vec2 cp2 = clipPolygon[j];
//		if (outputVertices.empty())
//			return false;
//
//		vector<b2Vec2> inputList = outputVertices;
//		outputVertices.clear();
//
//		b2Vec2 s = inputList[inputList.size() - 1];
//		for (int i = 0; i < inputList.size(); i++) {
//			b2Vec2 e = inputList[i];
//
//			if (!inside(cp1, cp2, e)) {
//				if (!inside(cp1, cp2, s)) {
//					outputVertices.push_back(intersection(cp1, cp2, s, e));
//				}
//				outputVertices.push_back(e);
//			}
//			else if (inside(cp1, cp2, s)) {
//				outputVertices.push_back(intersection(cp1, cp2, s, e));
//			}
//			s = e;
//		}
//		cp1 = cp2;
//	}
//	return !outputVertices.empty();
//}
