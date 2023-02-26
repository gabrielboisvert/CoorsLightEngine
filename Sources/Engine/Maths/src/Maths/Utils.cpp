#include "Maths/Utils.h"


const double Maths::radiansToDegrees(const double pRad)
{
	return pRad * (HALF_CIRCLE / PI);
}

const double Maths::degreesToRadians(const double pDeg)
{
	return pDeg * (PI / HALF_CIRCLE);
}

const float Maths::lerp(const float pA, const float pB, const float pT)
{
	Maths::clamp(0, 1, pT);
	return (pA + (pB - pA) * pT);
}

const float Maths::unLerp(const float pA, const float pB, const float pT)
{
	return Maths::clamp(((pT - pA) / (pB - pA)), 0, 1);
}

const float Maths::clamp(const float pVal, const float pMin, const float pMax)
{
	const float t = pVal < pMin ? pMin : pVal;
	return t > pMax ? pMax : t;
}

const float Maths::fmax(const float pA, const float pB)
{
	return (pA < pB) ? pB : pA;
}

const float Maths::fmin(const float pA, const float pB)
{
	return !(pB < pA) ? pA : pB;
}