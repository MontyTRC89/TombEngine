#ifndef MATH
#define MATH

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923
#define PI2 6.2831853071795864769252867665590057683943387987502116419498891846

float Luma(float3 color)
{
	// Use Rec.709 trichromat formula to get perceptive luma value
	return float((color.x * 0.2126f) + (color.y * 0.7152f) + (color.z * 0.0722f));
}

float3 Screen(float3 ambient, float3 tint)
{
	float luma = Luma(tint);

	float3 multiplicative = ambient * tint;
	float3 additive = ambient + tint;

	float r = lerp(multiplicative.x, additive.x, luma);
	float g = lerp(multiplicative.y, additive.y, luma);
	float b = lerp(multiplicative.z, additive.z, luma);

	return float3(r, g, b);
}

float LinearizeDepth(float depth, float nearPlane, float farPlane)
{
	return (2.0f * nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
}

float3 Mod289(float3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 Mod289(float4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 Permute(float4 x) {
	return Mod289(((x*34.0)+10.0)*x);
}

float4 TaylorInvSqrt(float4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float SimplexNoise(float3 v)
{ 
	const float2 C = float2(1.0/6.0, 1.0/3.0) ;
	const float4 D = float4(0.0, 0.5, 1.0, 2.0);

// First corner
	float3 i	= floor(v + dot(v, C.yyy) );
	float3 x0	= v - i + dot(i, C.xxx) ;

// Other corners
	float3 g = step(x0.yzx, x0.xyz);
	float3 l = 1.0 - g;
	float3 i1 = min( g.xyz, l.zxy );
	float3 i2 = max( g.xyz, l.zxy );

// x0 = x0 - 0.0 + 0.0 * C.xxx;
// x1 = x0 - i1  + 1.0 * C.xxx;
// x2 = x0 - i2  + 2.0 * C.xxx;
// x3 = x0 - 1.0 + 3.0 * C.xxx;
	float3 x1 = x0 - i1 + C.xxx;
	float3 x2 = x0 - i2 + C.yyy;// 2.0*C.x = 1/3 = C.y
	float3 x3 = x0 - D.yyy;		// -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
	i = Mod289(i); 
	float4	p = Permute( Permute( Permute( 
			i.z + float4(0.0, i1.z, i2.z, 1.0 )) +
			i.y + float4(0.0, i1.y, i2.y, 1.0 )) +
			i.x + float4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	float3 ns = n_ * D.wyz - D.xzx;

	float4 j = p - 49.0 * floor(p * ns.z * ns.z);// mod(p,7*7)

	float4 x_ = floor(j * ns.z);
	float4 y_ = floor(j - 7.0 * x_ );// mod(j,N)

	float4 x = x_ *ns.x + ns.yyyy;
	float4 y = y_ *ns.x + ns.yyyy;
	float4 h = 1.0 - abs(x) - abs(y);

	float4 b0 = float4( x.x, x.y, y.x, y.y );
	float4 b1 = float4( x.z, x.w, y.z, y.w );

	//float4 s0 = float4(lessThan(b0,0.0))*2.0 - 1.0;
	//float4 s1 = float4(lessThan(b1,0.0))*2.0 - 1.0;
	float4 s0 = floor(b0)*2.0 + 1.0;
	float4 s1 = floor(b1)*2.0 + 1.0;
	float4 sh = -step(h, float4(0.0f,0.0f,0.0f,0.0f));

	float4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
	float4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

	float3 p0 = float3(a0.xy,h.x);
	float3 p1 = float3(a0.zw,h.y);
	float3 p2 = float3(a1.xy,h.z);
	float3 p3 = float3(a1.zw,h.w);

//Normalise gradients
	float4 norm = TaylorInvSqrt(float4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

// Mix final noise value
	float4 m = max(0.5 - float4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 105.0 * dot( m*m, float4( dot(p0,x0), dot(p1,x1), 
									 dot(p2,x2), dot(p3,x3) ) );
}

float3 NormalNoise(float3 v, float3 i, float3 n)
{
//the resulting vector
	float3 r = float3(.0f,.0f,.0f);

//interpolates lerp(v,i,dot(n,i))
//v, which is the reference of the pixel and
//i, a vector perturbed with SimpleNoise
//the threshold dot(n,i) is based on a dot product between
//n, the normal and each tuple from it (xy, xz and yz)
//and the same for the vector from the noise
	r.x = lerp(v.x, i.x, dot(n.xy,i.xy))*.9f;
	r.y = lerp(v.y, i.y, dot(n.xz,i.xz))*.75f;
	r.z = lerp(v.z, i.z, dot(n.yz,i.yz))*.8f;

//c, is a threshold based on the tuples of the reference pixel
	float c = dot(v.xy,v.yz);

//returns the perturbed pixel based on the reference pixel
//and the resulting vector, with a threshold c attenuated by 2/5 times
	return lerp(v,r,c*.3f);
}
#endif // MATH
