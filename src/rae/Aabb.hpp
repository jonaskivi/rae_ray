#pragma once

#include <glm/glm.hpp>
using glm::vec3;

namespace Rae
{

class Ray;

class Aabb
{
public:
	Aabb()
	: m_min(FLT_MAX, FLT_MAX, FLT_MAX),
	m_max(FLT_MIN, FLT_MIN, FLT_MIN)
	{
	}

	Aabb(const vec3& setMin, const vec3& setMax)
	: m_min(setMin),
	m_max(setMax)
	{
	}

	void clear()
	{
		m_min = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		m_max = vec3(FLT_MIN, FLT_MIN, FLT_MIN);
	}

	bool valid()
	{
		if (m_min.x <= m_max.x
			&& m_min.y <= m_max.y
			&& m_min.z <= m_max.z)
			return true;
		return false;
	}

	void init(const Aabb& left, const Aabb& right);
	void grow(const Aabb& set);
	void grow(vec3 set);

	vec3 dimensions()
	{
		return m_max - m_min;
	}

	const vec3& min() const { return m_min; }
	const vec3& max() const { return m_max; }

	bool hit(const Ray& ray, float minDistance, float maxDistance) const;

protected:
	vec3 m_min;
	vec3 m_max;
};

}
