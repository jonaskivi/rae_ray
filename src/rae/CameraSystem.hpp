#pragma once

#include <vector>
#include <functional>

#include "Entity.hpp"

#include "Camera.hpp"
#include "System.hpp"

namespace Rae
{

class Input;

class CameraSystem : public System
{
public:
	CameraSystem(Input& input);

	void update(double time, double delta_time, std::vector<Entity>& entities) override;

	void onMouseEvent(const Input& input);
	void onKeyEvent(const Input& input);

	void setNeedsUpdate() { m_camera.setNeedsUpdate(); }
	void setAspectRatio(float aspect) { m_camera.setAspectRatio(aspect); }

	const Camera&    getCurrentCamera() const { return m_camera; }
	Camera&          getCurrentCamera()       { return m_camera; }

	void connectCameraChangedEventHandler(std::function<void(const Camera&)> handler);

private:
	Input& m_input;
	Camera m_camera;

	void emitCameraChangedEvent();
	std::vector<std::function<void(const Camera&)>> cameraChangedEvent;
};

}
