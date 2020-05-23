#include "framework.h"
#include "trmath.h"
#include "smoke.h"
#include "room.h"
#include "control.h"
#include "level.h"
std::array<SmokeParticle, 128> SmokeParticles;


void UpdateSmokeParticles()
{
	for (int i = 0; i < SmokeParticles.size(); i++) {
		SmokeParticle& s = SmokeParticles[i];
		if (!s.active)continue;
		s.age += 1;
		if (s.age > s.life) {
			s.active = false;
			continue;
		}
		s.velocity.y + s.gravity;
		if (s.terminalVelocity != 0) {
			float velocityLength = s.velocity.Length();
			if (velocityLength > s.terminalVelocity) {
				s.velocity *= (s.terminalVelocity / velocityLength);
			}
		}
		s.position += s.velocity;
		if (s.affectedByWind) {
			if (Rooms[s.room].flags & ENV_FLAG_WIND) {
				s.position.x += SmokeWindX/2;
				s.position.z += SmokeWindZ/2;
			}
		}
		float normalizedLife = s.age / s.life;
		s.size = lerp(s.sourceSize, s.destinationSize, normalizedLife);
		s.angularVelocity *= s.angularDrag;
		s.rotation += s.angularVelocity;
		s.color = DirectX::SimpleMath::Vector4::Lerp(s.sourceColor, s.destinationColor, normalizedLife);	
	}
}

void TriggerFlareSmoke(const DirectX::SimpleMath::Vector3& pos, DirectX::SimpleMath::Vector3& direction,int age,int room) {
	using namespace DirectX::SimpleMath;
	SmokeParticle& const s = getFreeSmokeParticle();
	s = {};
	s.position = pos;
	s.age = 0;
	constexpr float d = 0.2f;
	Vector3 randomDir = Vector3(frandMinMax(-d, d), frandMinMax(-d, d), frandMinMax(-d, d));
	Vector3 dir;
	(direction + randomDir).Normalize(dir);
	s.velocity = dir *frandMinMax(7,9);
	s.gravity = 1;
	s.friction = frandMinMax(0.7f,0.85f);
	s.sourceColor = Vector4(1, 131/255.0f, 100/255.0f, 1);
	s.destinationColor = Vector4(0, 0, 0, 0);
	s.life = frandMinMax(25, 35);
	s.angularVelocity = frandMinMax(-0.3f, 0.3f);
	s.angularDrag = 0.98f;
	s.sourceSize = age > 4 ? frandMinMax(16, 24) : frandMinMax(100	, 128);
	s.destinationSize = age > 4 ? frandMinMax(160, 200) : frandMinMax(256, 300);
	s.affectedByWind = true;
	s.active = true;
	s.room = room;
}

SmokeParticle& getFreeSmokeParticle()
{
	for (int i = 0; i < SmokeParticles.size(); i++) {
		if (!SmokeParticles[i].active)
			return SmokeParticles[i];
	}
	return SmokeParticles[0];
}
