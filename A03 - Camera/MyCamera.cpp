/// Assignment 3 - Camera
/// TJ Wolschon
#include "MyCamera.h"
using namespace Simplex;

//Accessors
void Simplex::MyCamera::SetPosition(vector3 a_v3Position) { m_v3Position = a_v3Position; }
vector3 Simplex::MyCamera::GetPosition(void) { return m_v3Position; }
void Simplex::MyCamera::SetTarget(vector3 a_v3Target) { m_v3Target = a_v3Target; }
vector3 Simplex::MyCamera::GetTarget(void) { return m_v3Target; }
void Simplex::MyCamera::SetAbove(vector3 a_v3Above) { m_v3Above = a_v3Above; }
vector3 Simplex::MyCamera::GetAbove(void) { return m_v3Above; }
void Simplex::MyCamera::SetPerspective(bool a_bPerspective) { m_bPerspective = a_bPerspective; }
void Simplex::MyCamera::SetFOV(float a_fFOV) { m_fFOV = a_fFOV; }
void Simplex::MyCamera::SetResolution(vector2 a_v2Resolution) { m_v2Resolution = a_v2Resolution; }
void Simplex::MyCamera::SetNearFar(vector2 a_v2NearFar) { m_v2NearFar = a_v2NearFar; }
void Simplex::MyCamera::SetHorizontalPlanes(vector2 a_v2Horizontal) { m_v2Horizontal = a_v2Horizontal; }
void Simplex::MyCamera::SetVerticalPlanes(vector2 a_v2Vertical) { m_v2Vertical = a_v2Vertical; }
matrix4 Simplex::MyCamera::GetProjectionMatrix(void) { return m_m4Projection; }
matrix4 Simplex::MyCamera::GetViewMatrix(void) { CalculateViewMatrix(); return m_m4View; }

Simplex::MyCamera::MyCamera()
{
	Init(); //Init the object with default values
}

Simplex::MyCamera::MyCamera(vector3 a_v3Position, vector3 a_v3Target, vector3 a_v3Upward)
{
	Init(); //Initialize the object
	SetPositionTargetAndUpward(a_v3Position, a_v3Target, a_v3Upward); //set the position, target and upward
}

Simplex::MyCamera::MyCamera(MyCamera const& other)
{
	m_v3Position = other.m_v3Position;
	m_v3Target = other.m_v3Target;
	m_v3Above = other.m_v3Above;

	m_bPerspective = other.m_bPerspective;

	m_fFOV = other.m_fFOV;

	m_v2Resolution = other.m_v2Resolution;
	m_v2NearFar = other.m_v2NearFar;

	m_v2Horizontal = other.m_v2Horizontal;
	m_v2Vertical = other.m_v2Vertical;

	m_m4View = other.m_m4View;
	m_m4Projection = other.m_m4Projection;
}

MyCamera& Simplex::MyCamera::operator=(MyCamera const& other)
{
	if (this != &other)
	{
		Release();
		SetPositionTargetAndUpward(other.m_v3Position, other.m_v3Target, other.m_v3Above);
		MyCamera temp(other);
		Swap(temp);
	}
	return *this;
}

void Simplex::MyCamera::Init(void)
{
	ResetCamera();
	CalculateProjectionMatrix();
	CalculateViewMatrix();
	//No pointers to initialize here
}

void Simplex::MyCamera::Release(void)
{
	//No pointers to deallocate
}

void Simplex::MyCamera::Swap(MyCamera & other)
{
	std::swap(m_v3Position, other.m_v3Position);
	std::swap(m_v3Target, other.m_v3Target);
	std::swap(m_v3Above, other.m_v3Above);

	std::swap(m_bPerspective, other.m_bPerspective);

	std::swap(m_fFOV, other.m_fFOV);

	std::swap(m_v2Resolution, other.m_v2Resolution);
	std::swap(m_v2NearFar, other.m_v2NearFar);

	std::swap(m_v2Horizontal, other.m_v2Horizontal);
	std::swap(m_v2Vertical, other.m_v2Vertical);

	std::swap(m_m4View, other.m_m4View);
	std::swap(m_m4Projection, other.m_m4Projection);
}

Simplex::MyCamera::~MyCamera(void)
{
	Release();
}

void Simplex::MyCamera::ResetCamera(void)
{
	m_v3Position = vector3(0.0f, 0.0f, 10.0f); //Where my camera is located
	m_v3Target = vector3(0.0f, 0.0f, 0.0f); //What I'm looking at
	m_v3Above = vector3(0.0f, 1.0f, 0.0f); //What is above the camera

	m_bPerspective = true; //perspective view? False is Orthographic

	m_fFOV = 45.0f; //Field of View

	m_v2Resolution = vector2(1280.0f, 720.0f); //Resolution of the window
	m_v2NearFar = vector2(0.001f, 1000.0f); //Near and Far planes

	m_v2Horizontal = vector2(-5.0f, 5.0f); //Orthographic horizontal projection
	m_v2Vertical = vector2(-5.0f, 5.0f); //Orthographic vertical projection

	CalculateProjectionMatrix();
	CalculateViewMatrix();
}

void Simplex::MyCamera::SetPositionTargetAndUpward(vector3 a_v3Position, vector3 a_v3Target, vector3 a_v3Upward)
{
	m_v3Position = a_v3Position;
	m_v3Target = a_v3Target;

	m_v3Above = a_v3Position + glm::normalize(a_v3Upward);

	//Calculate the Matrix
	CalculateProjectionMatrix();
}

void Simplex::MyCamera::CalculateViewMatrix(void)
{
	//Calculate the look at most of your assignment will be reflected in this method
	m_m4View = glm::lookAt(m_v3Position, m_v3Target, glm::normalize(m_v3Above - m_v3Position)); //position, target, upward
}

void Simplex::MyCamera::CalculateProjectionMatrix(void)
{
	float fRatio = m_v2Resolution.x / m_v2Resolution.y;
	if (m_bPerspective) //perspective
	{
		m_m4Projection = glm::perspective(m_fFOV, fRatio, m_v2NearFar.x, m_v2NearFar.y);
	}
	else //Orthographic
	{
		m_m4Projection = glm::ortho(m_v2Horizontal.x * fRatio, m_v2Horizontal.y * fRatio, //horizontal
			m_v2Vertical.x, m_v2Vertical.y, //vertical
			m_v2NearFar.x, m_v2NearFar.y); //near and far
	}
}

void MyCamera::MoveForward(float a_fDistance)
{
	//get our forward vector in local space
	vector3 forward = m_v3Target - m_v3Position;

	//move in the direction of our forward
	m_v3Position += forward * a_fDistance;
	
	//update our target in the direction of our forward
	m_v3Target += forward * a_fDistance;

	//update the above in direction of our forward
	m_v3Above += forward * a_fDistance;
}

void MyCamera::MoveVertical(float a_fDistance)
{
	//move in the y direction
	m_v3Position += vector3(0.f, -a_fDistance, 0.f);
	m_v3Target += vector3(0.f, -a_fDistance, 0.f);
	m_v3Above += vector3(0.f, -a_fDistance, 0.f);
}

void MyCamera::MoveSideways(float a_fDistance)
{
	vector3 forward = m_v3Target - m_v3Position;
	
	//get our right vector, the cross between y axis and forward vector
	vector3 right = glm::cross(forward, AXIS_Y);

	//move in the direction of our right vector
	m_v3Position += right * -a_fDistance;
	m_v3Target += right * -a_fDistance;
	m_v3Above += right * -a_fDistance;
}

//rotate target around camera right axis
void MyCamera::ChangePitch(float angle)
{
	//get our forward vector which is just our target moved to the origin
	vector3 forward = m_v3Target - m_v3Position;
	//std::cout << "fwd x: " << forward.x << "\ty:" << forward.y << "\tz:" << forward.z << std::endl;
	
	//find our right vector which is the cross product of our forward and the y axis
	vector3 right = glm::cross(forward, AXIS_Y);

	//setup a new target vector to be rotated
	vector3 tgt = forward;

	//using the given angle, setup a quaternion on the right axis
	quaternion quat = glm::angleAxis(glm::radians(angle * m_fCameraSensitivity), right);

	//rotate our vector in the orientation of that quaternion, and normalize it
	tgt = glm::normalize(quat * tgt);

	//move it back in front of us
	tgt = tgt + GetPosition();

	//set it as the target to be looked at
	SetTarget(tgt);
}
//rotate target around camera up axis
void MyCamera::ChangeYaw(float angle)
{
	vector3 up = AXIS_Y;

	//setup a target vector which is the same as the forward vector at the origin
	vector3 tgt = GetTarget() - GetPosition();

	//using the given angle, setup a quaternion on the up axis
	quaternion quat = glm::angleAxis(glm::radians(angle * m_fCameraSensitivity), up);

	//rotate our vector in the orientation of that quaternion, and normalize it
	tgt = glm::normalize(quat * tgt);

	//move it back in front of us
	tgt = tgt + GetPosition();

	//set it as the target to be looked at
	SetTarget(tgt);
}

/*void MyCamera::RotateCamera(float yaw, float pitch)
{
	//quaternion currRot;
	//currRot *= glm::angleAxis(glm::radians(pitch), vector3(1.f, 0.f, 0.f));
	//currRot *= glm::angleAxis(glm::radians(yaw), vector3(0.f, 1.f, 0.f));
	//currRot = glm::normalize(currRot);

	//vector3 newTarget = m_v3Target - m_v3Position;
	//newTarget = currRot * newTarget;
	//std::cout << "x: " << newTarget.x << " \ty:" << newTarget.y << " \tz:" << newTarget.z << std::endl;
	//vector3 right = glm::cross(newTarget, vector3(0, -1, 0));
	//vector3 up = glm::cross(newTarget, right);
	//newTarget += m_v3Position;

	//SetPositionTargetAndUpward(m_v3Position, newTarget, up);

	vector3 direction(
		cos(pitch) * sin(yaw),
		sin(pitch),
		cos(pitch) * cos(yaw)
	);

	// Right vector
	vector3 right = vector3(
		sin(yaw - 3.14f / 2.0f),
		0,
		cos(yaw - 3.14f / 2.0f)
	);

	vector3 up = glm::cross(right, direction);

	SetTarget(direction);
	m_v3Above = m_v3Position + glm::normalize(up);
}*/