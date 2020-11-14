#include "framework.h"
#include "SimpleParticle.h"
#include "items.h"
#include "trmath.h"
#include <Specific\setup.h>
#include "prng.h"
using namespace T5M::Math::Random;

namespace T5M::Effects{
	std::array<SimpleParticle, 15> simpleParticles;
	SimpleParticle& getFreeSimpleParticle()
	{
		for(auto& p : simpleParticles)
			if(!p.active)
				return p;
		return simpleParticles[0];
	}
	void TriggerSnowmobileSnow(ITEM_INFO* snowMobile)
	{
		float angle = TO_RAD(snowMobile->pos.yRot);
		const float angleVariation = generateFloat(-10, 10) * RADIAN;
		float x = std::sin(angle + angleVariation);
		float z = std::cos(angle + angleVariation);
		x = x* -500 + snowMobile->pos.xPos;
		z = z* -500 + snowMobile->pos.zPos;
		SimpleParticle& p = getFreeSimpleParticle();
		p = {};
		p.active = true;
		p.life = generateFloat(8, 14);
		p.room = snowMobile->roomNumber;
		p.ageRate = generateFloat(0.9, 1.3);
		float size = generateFloat(96, 128);
		p.worldPosition = {x,float(snowMobile->pos.yPos) - size/2,z};
		p.sequence = ID_SKIDOO_SNOW_TRAIL_SPRITES;
		p.size = generateFloat(256, 512);

	}

	void TriggerSpeedboatFoam(ITEM_INFO* boat)
	{
		for (float i = -0.5;i < 1; i+=1) {
			float angle = TO_RAD(boat->pos.yRot);
			float angleVariation = i*2*10 * RADIAN;
			float x = std::sin(angle + angleVariation);
			float z = std::cos(angle + angleVariation);
			x = x * -700 + boat->pos.xPos;
			z = z * -700 + boat->pos.zPos;
			SimpleParticle& p = getFreeSimpleParticle();
			p = {};
			p.active = true;
			p.life = generateFloat(5, 9);
			p.room = boat->roomNumber;
			p.ageRate = generateFloat(0.9, 1.3);
			float size = generateFloat(96, 128);
			p.worldPosition = { x,float(boat->pos.yPos) - size / 2,z };
			p.sequence = ID_MOTOR_BOAT_FOAM_SPRITES;
			p.size = generateFloat(256, 512);
		}
		
	}

	void updateSimpleParticles()
	{
		for(auto& p : simpleParticles){
			if(!p.active)
				continue;
			p.age+= p.ageRate;
			if(p.life < p.age)
				p.active = false;
			int numSprites = -Objects[p.sequence].nmeshes - 1;
			float normalizedAge = p.age / p.life;
			p.sprite = lerp(0, numSprites, normalizedAge);
		}
	}
}
