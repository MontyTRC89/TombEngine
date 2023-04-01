#pragma once

namespace TEN::Renderer 
{
	struct RendererVertex 
	{
		Vector3 Position = Vector3::Zero;
		Vector3 Normal	 = Vector3::Zero;
		Vector2 UV		 = Vector2::Zero;
		Vector4 Color	 = Vector4::Zero;
		Vector3 Tangent	 = Vector3::Zero;

		unsigned int AnimationFrameOffset = 0;
		Vector4		 Effects			  = Vector4::Zero;
		float		 Bone				  = 0.0f;
		unsigned int IndexInPoly		  = 0;
		unsigned int OriginalIndex		  = 0;
		unsigned int Hash				  = 0;
	};
}
