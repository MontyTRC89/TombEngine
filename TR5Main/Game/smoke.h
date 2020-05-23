#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <array>
struct SmokeParticle {
	DirectX::SimpleMath::Vector4 sourceColor;
	DirectX::SimpleMath::Vector4 destinationColor;
	DirectX::SimpleMath::Vector4 color;
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 velocity;
	int room;
	float gravity;
	float friction;
	float sourceSize;
	float destinationSize;
	float size;
	float age;
	float life;
	float angularVelocity;
	float angularDrag;
	float rotation;
	float terminalVelocity;
	bool affectedByWind;
	bool active;
};

extern std::array<SmokeParticle, 128> SmokeParticles;

void UpdateSmokeParticles();
void TriggerFlareSmoke(const DirectX::SimpleMath::Vector3& pos, DirectX::SimpleMath::Vector3& direction,int age,int room);
SmokeParticle& getFreeSmokeParticle();