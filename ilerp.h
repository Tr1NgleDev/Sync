#pragma once

#include <glm/glm.hpp>

float deltaRatio(float ratio, double dt, double targetDelta)
{
	const double rDelta = dt / (1.0 / (1.0 / targetDelta));
	const double s = 1.0 - ratio;

	return (float)(1.0 - pow(s, rDelta));
}

float deltaRatio(float ratio, double dt)
{
	return deltaRatio(ratio, dt, 1.0 / 100.0);
}

float lerp(float a, float b, float ratio, bool clampRatio = true)
{
	if (clampRatio)
		ratio = glm::clamp(ratio, 0.f, 1.f);
	return a + (b - a) * ratio;
}

float ilerp(float a, float b, float ratio, double dt, bool clampRatio = true)
{
	return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
}

glm::vec4 ilerp(glm::vec4 a, glm::vec4 b, float ratio, double dt, bool clampRatio = true)
{
	return
	{
		ilerp(a.x, b.x, ratio, dt),
		ilerp(a.y, b.y, ratio, dt),
		ilerp(a.z, b.z, ratio, dt),
		ilerp(a.w, b.w, ratio, dt),
	};
}
