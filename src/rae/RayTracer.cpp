#include "RayTracer.hpp"

#include <iostream>	
#include <vector>
#include <string>

#include <glm/glm.hpp>
using glm::vec3;
using glm::dot;

#include "core/Utils.hpp"
#include "Random.hpp"

#include "CameraSystem.hpp"
#include "Material.hpp"
#include "Sphere.hpp"
#include "Mesh.hpp"

using namespace Rae;

ImageBuffer::ImageBuffer()
: width(0),
height(0),
imageId(-1) // init to invalid value.
{
}

ImageBuffer::ImageBuffer(int setWidth, int setHeight)
: width(setWidth),
height(setHeight),
imageId(-1) // init to invalid value.
{
	init();
}

void ImageBuffer::init(int setWidth, int setHeight)
{
	width = setWidth;
	height = setHeight;

	init();
}

void ImageBuffer::init()
{
	if (width == 0 || height == 0)
	{
		assert(0);
		return;
	}

	if (colorData.size() > 0)
		colorData.clear();
	if (data.size() > 0)
		data.clear();

	colorData.reserve(width * height);
	for (int i = 0; i < width * height; ++i)
	{
		colorData.push_back(vec3(0.5f, 0.5f, 0.5f));
	}	
	data.reserve(width * height * channels);
	for (int i = 0; i < width * height; ++i)
	{
		data.push_back(0);
		data.push_back(0);
		data.push_back(0);
		data.push_back(255);
	}
}

void ImageBuffer::createImage(NVGcontext* vg)
{
	if (imageId == -1 && vg != nullptr)
	{
		//std::cout << "Creating image " << width << "x" << height << "\n";
		imageId = nvgCreateImageRGBA(vg, width, height, /*imageFlags*/0, &data[0]);
	}
	else
	{
		std::cout << "Failed to create an image " << width << "x" << height << "\n";
		if (imageId != -1)
			std::cout << "imageId was not -1. It was " << imageId << "\n";
		if (vg == nullptr)
			std::cout << "vg was null.\n";
		assert(imageId == -1);
		assert(vg != nullptr);
	}
}

vec3 pow(const vec3& color, float power)
{
	vec3 result;
	result.r = glm::pow(color.r, power);
	result.g = glm::pow(color.g, power);
	result.b = glm::pow(color.b, power);
	return result;
}

vec3 gammaCorrectionAnd255(const vec3& linear)
{
	const float gammaMul = 1.0f/2.2f;
	return 255.99f * glm::clamp( pow(linear, gammaMul), 0.0f, 1.0f);
}

void ImageBuffer::update8BitImageBuffer(NVGcontext* vg)
{
	// update 8 bit image buffer
	{
		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				const vec3& linear = colorData[(j*width)+i];

				vec3 color = gammaCorrectionAnd255(linear);

				data[(j*width*channels) + (i*channels) + 0] = int8_t(color.r);
				data[(j*width*channels) + (i*channels) + 1] = int8_t(color.g);
				data[(j*width*channels) + (i*channels) + 2] = int8_t(color.b);
			}
		}

		nvgUpdateImage(vg, imageId, &data[0]);
	}
}

//----------------------------------------------------------------------------------------------------------------------

RayTracer::RayTracer(CameraSystem& cameraSystem)
: m_world(4),
m_cameraSystem(cameraSystem)
{
	m_smallBuffer.init(300, 150);
	m_bigBuffer.init(1920, 1080);

	m_buffer = &m_smallBuffer;

	createSceneOne(m_world);
	//createSceneFromBook(m_world);

	using std::placeholders::_1;
	m_cameraSystem.connectCameraChangedEventHandler(std::bind(&RayTracer::onCameraChanged, this, _1));
}

RayTracer::~RayTracer()
{
}

void ImageBuffer::clear()
{
	std::fill(colorData.begin(), colorData.end(), vec3(0,0,0));
	//std::fill(data.begin(), data.end(), 0);
}

void RayTracer::createSceneOne(HitableList& world, bool loadBunny)
{
	Camera& camera = m_cameraSystem.getCurrentCamera();

	camera.setFieldOfViewDeg(44.6f);

	//camera.setPosition(vec3(0.698890f, 1.275992f, 6.693169f));
	//camera.setYaw(Math::toRadians(188.0f));
	//camera.setPitch(Math::toRadians(-7.744f));
	//camera.setAperture(0.3f);
	//camera.setFocusDistance(7.6f);

	camera.setPosition(vec3(-0.16f, 2.9664f, 14.8691f));
	camera.setYaw(Math::toRadians(178.560333f));
	camera.setPitch(Math::toRadians(-10.8084f));
	camera.setAperture(0.07f);
	camera.setFocusDistance(14.763986f);

	// A big light
	world.add(
		new Sphere(vec3(0.0f, 6.0f, -1.0f), 2.0f,
		new Light(vec3(4.0f, 4.0f, 4.0f)))
		);

	// A small light
	world.add(
		new Sphere(vec3(0.85, 0.3, -0.15f), 0.1f,
		new Light(vec3(16.0f, 16.0f, 16.0f)))
		);

	// A ball
	world.add(
		new Sphere(vec3(0, 0, -1), 0.5f,
		new Lambertian(vec3(0.8f, 0.3f, 0.3f)))
		);
	// The planet
	world.add(
		new Sphere(vec3(0, -100.5f, -1), 100.0f,
		new Lambertian(vec3(0.0f, 0.7f, 0.8f)))
		);
	
	// Metal balls
	world.add(
		new Sphere(vec3(1, 0, -1), 0.5f,
		new Metal(vec3(0.8f, 0.6f, 0.2f), /*roughness*/0.0f))
		);
	world.add(
		new Sphere(vec3(-0.5f, 0.65f, -1), 0.4f,
		new Metal(vec3(0.8f, 0.4f, 0.8f), /*roughness*/0.3f))
		);
	// Dielectric
	world.add(
		new Sphere(vec3(-1, 0, -1), 0.5f,
		new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f))
		);
	world.add(
		new Sphere(vec3(-3.15f, 0.1f, -5), 0.6f,
		new Lambertian(vec3(0.05f, 0.2f, 0.8f)))
		);

	///////////////////

	auto bunny = new Mesh(0);
	if (loadBunny)
		bunny->loadModel("./data/models/bunny.obj");
	else bunny->generateBox();

	world.add(bunny);

	m_tree.init(world.list(), 0, 0);
}

void RayTracer::createSceneFromBook(HitableList& list)
{
	Camera& camera = m_cameraSystem.getCurrentCamera();

	camera.setPosition(vec3(16.857f, 2.0f, 6.474f));
	camera.setYaw(Math::toRadians(247.8f));
	camera.setPitch(Math::toRadians(-4.762f));
	camera.setAperture(0.1f);
	camera.setFocusDistance(17.29f);

	list.add( new Sphere(vec3(0,-1000,0), 1000, new Lambertian(vec3(0.5, 0.5, 0.5))) );

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			float choose_mat = getRandom();
			vec3 center(a + 0.9f * getRandom(), 0.2f, b + 0.9f * getRandom());
			if ((center-vec3(4,0.2,0)).length() > 0.9f)
			{ 
				if (choose_mat < 0.8f)
				{
					// diffuse
					list.add( new Sphere(center, 0.2f, new Lambertian(vec3( getRandom()*getRandom(), getRandom()*getRandom(), getRandom()*getRandom()))));
				}
				else if (choose_mat < 0.95f)
				{
					// metal
					list.add( new Sphere(center, 0.2f,
							new Metal(vec3(0.5f*(1.0f + getRandom()), 0.5f*(1.0f + getRandom()), 0.5f*(1.0f + getRandom())), /*roughness*/ 0.5f*getRandom())));
				}
				else
				{
					// glass
					list.add( new Sphere(center, 0.2f, new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f)) );
				}
			}
		}
	}

	list.add( new Sphere(vec3(0, 1, 0), 1.0, new Dielectric(vec3(0.8f, 0.5f, 0.3f), /*refractive_index*/1.5f)) );
	list.add( new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(vec3(0.0, 0.2, 0.9))) );
	list.add( new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0)) );

	m_tree.init(list.list(), 0, 0);
}

void RayTracer::showScene(int number)
{
	if (number == 1)
	{
		clearScene();
		createSceneOne(m_world, false);
	}

	if (number == 2)
	{
		clearScene();
		createSceneOne(m_world, true);
	}

	if (number == 3)
	{
		clearScene();
		createSceneFromBook(m_world);
	}
}

void RayTracer::clearScene()
{
	m_world.clear();
	m_cameraSystem.setNeedsUpdate();
	clear();
}

void RayTracer::onCameraChanged(const Camera& camera)
{
	if (camera.shouldWeAutoFocus())
		autoFocus();

	clear();
}

void RayTracer::clear()
{
	m_buffer->clear();
	m_currentSample = 0;
	m_totalRayTracingTime = -1.0;
	m_startTime = -1.0f;
}

void RayTracer::setNanovgContext(NVGcontext* setVg)
{
	assert(setVg != NULL);

	m_vg = setVg;

	m_smallBuffer.createImage(m_vg);
	m_bigBuffer.createImage(m_vg);
}

std::string toString(const HitRecord& record)
{
	return "t: " + std::to_string(record.t) + ", "
		+ "point: (" + std::to_string(record.point.x) + ", "
		+ std::to_string(record.point.y) + ", "
		+ std::to_string(record.point.z) + ")";
}

void RayTracer::autoFocus()
{
	// Get a ray to middle of the screen and focus there
	Camera& camera = m_cameraSystem.getCurrentCamera();
	Ray ray = camera.getExactRay(0.5f, 0.5f);
	HitRecord record;
	if (m_tree.hit(ray, 0.001f, FLT_MAX, record))
	{
		debugHitRecord = record;
		camera.animateFocusPosition(record.point, camera.focusSpeed());
	}
}

vec3 RayTracer::rayTrace(const Ray& ray, Hitable& world, int depth)
{
	Camera& camera = m_cameraSystem.getCurrentCamera();
	HitRecord record;
	if (m_tree.hit(ray, 0.001f, rayMaxLength(), record))
	{
		// Visualize focus distance with a line
		if (m_isVisualizeFocusDistance)
		{
			float hitDistance = glm::length(record.point - camera.position());
			if (Utils::isEqual(camera.focusDistance(), hitDistance, 0.01f) == true)
			{
				return vec3(0,1,1); // cyan line
			}
		}

		// Normal raytracing
		if (isFastMode() == false)
		{
			Ray scattered;
			vec3 attenuation;
			vec3 emitted = record.material->emitted(record.point);

			if (depth < m_bouncesLimit && record.material->scatter(ray, record, attenuation, scattered))
			{
				return emitted + attenuation * rayTrace(scattered, world, depth + 1);
			}
			else
			{
				return emitted;
			}
		}
		else // FastMode returns just the material color
		{
			return record.material->albedo;
		}
	}
	return sky(ray);
}

vec3 RayTracer::sky(const Ray& ray)
{
	vec3 unitDirection = glm::normalize( ray.direction() );
	float t = 0.5f * (unitDirection.y + 1.0f);
	return (1.0f - t) * vec3(0.3f, 0.4f, 1.0f) + t * vec3(0.7f, 0.8f, 1.0f);
}

//#define RENDER_ALL_AT_ONCE

void RayTracer::update(double time, double deltaTime, std::vector<Entity>& entities)
{
	/*
	Old time based switch buffers system:

	if (switchTime > 0.0f)
	{
		if (m_buffer != &m_bigBuffer
			&&
			(total_ray_tracing_time >= switchTime
			||
			current_sample >= samples_limit))
		{
			m_buffer = &m_bigBuffer;
			m_buffer->clear();
			current_sample = 0;
		}
	}
	*/

	if (m_startTime == -1.0f)
		m_startTime = time;

	if (m_totalRayTracingTime == -1.0f)
		m_totalRayTracingTime = time;

	#ifdef RENDER_ALL_AT_ONCE
		renderAllAtOnce(time);
	#else

		renderSamples(time, deltaTime);
		if (m_currentSample <= m_samplesLimit) // do once more than render
		{
			updateImageBuffer();
		}
	#endif
}

void RayTracer::toggleBufferQuality()
{
	if (m_buffer == &m_smallBuffer)
	{
		m_buffer = &m_bigBuffer;
	}
	else m_buffer = &m_smallBuffer;
	clear();
}

float RayTracer::rayMaxLength()
{
	if (isFastMode() == false)
		return FLT_MAX;
	else return 5.0f;
}

void RayTracer::plusBounces(int delta)
{
	m_bouncesLimit += delta;
	m_bouncesLimit = std::max(0, m_bouncesLimit);
	m_bouncesLimit = std::min(5000, m_bouncesLimit);
}

void RayTracer::minusBounces(int delta)
{
	m_bouncesLimit -= delta;
	m_bouncesLimit = std::max(0, m_bouncesLimit);
	m_bouncesLimit = std::min(5000, m_bouncesLimit);
}

void RayTracer::renderAllAtOnce(double time)
{
	// timings for 100 samples at 500x250:
	// 14.791584 s
	// 14.715808 s
	// 14.710577 s

	if (m_currentSample < m_samplesLimit)
	{
		Camera& camera = m_cameraSystem.getCurrentCamera();
		m_startTime = time;

		for (int j = 0; j < m_buffer->height; ++j)
		{
			for (int i = 0; i < m_buffer->width; ++i)
			{
				vec3 color;

				for (int sample = 0; sample < m_samplesLimit; sample++)
				{
					float u = float(i + drand48()) / float(m_buffer->width);
					float v = float(j + drand48()) / float(m_buffer->height);
					
					Ray ray = camera.getRay(u, v);
					color += rayTrace(ray, m_world, 0);
				}

				color /= float(m_samplesLimit);

				m_buffer->colorData[(j * m_buffer->width) + i] = color;
			}
		}
		
		m_currentSample = m_samplesLimit;
	}
	else if (m_currentSample == m_samplesLimit)
	{
		updateImageBuffer();

		// do only once:
		m_totalRayTracingTime = time - m_startTime;

		m_currentSample++;
	}
}

void RayTracer::renderSamples(double time, double deltaTime)
{
	// timings for 100 samples at 500x250:
	// 15.426324 s
	// 15.402015 s
	// 15.347182 s

	if (m_currentSample < m_samplesLimit)
	{
		Camera& camera = m_cameraSystem.getCurrentCamera();
		m_totalRayTracingTime = time - m_startTime;

		for (int j = 0; j < m_buffer->height; ++j)
		{
			for (int i = 0; i < m_buffer->width; ++i)
			{
				float u = float(i + drand48()) / float(m_buffer->width);
				float v = float(j + drand48()) / float(m_buffer->height);

				Ray ray = camera.getRay(u, v);
				vec3 color = rayTrace(ray, m_world, 0);

				//http://stackoverflow.com/questions/22999487/update-the-average-of-a-continuous-sequence-of-numbers-in-constant-time
				// add to average
				m_buffer->colorData[(j * m_buffer->width) + i]
					= (float(m_currentSample) * m_buffer->colorData[(j * m_buffer->width) + i] + color) / float(m_currentSample + 1);
			}
		}
		
		m_currentSample++;
	}
}

void RayTracer::updateImageBuffer()
{
	m_buffer->update8BitImageBuffer(m_vg);
}

void RayTracer::renderNanoVG(NVGcontext* vg, float x, float y, float w, float h)
{
	ImageBuffer& readBuffer = imageBuffer();

	nvgSave(vg);

	//override the given parameters and reuse w and h...
	/*
	x = -g_rae->screenHalfWidthP();
	y = -g_rae->screenHalfHeightP();
	w = g_rae->screenWidthP();
	h = g_rae->screenHeightP();
	*/

	m_imgPaint = nvgImagePattern(vg, x, y, w, h, 0.0f, readBuffer.imageId, 1.0f);
	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillPaint(vg, m_imgPaint);
	nvgFill(vg);

	// Text
	if (m_isInfoText)
	{
		Camera& camera = m_cameraSystem.getCurrentCamera();

		nvgFontFace(vg, "sans");

		nvgFontSize(vg, 18.0f);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
		nvgFillColor(vg, nvgRGBA(128, 128, 128, 192));
	
		float vertPos = 200.0f;

		std::string samplesStr = "Samples: " + std::to_string(m_currentSample);
		nvgText(vg, 10.0f, vertPos, samplesStr.c_str(), nullptr); vertPos += 20.0f;

		std::string samplesLimitStr = "/" + std::to_string(m_samplesLimit);
		nvgText(vg, 10.0f, vertPos, samplesLimitStr.c_str(), nullptr); vertPos += 20.0f;

		std::string totalTimeStr = "Time: " + std::to_string(m_totalRayTracingTime) + " s";
		nvgText(vg, 10.0f, vertPos, totalTimeStr.c_str(), nullptr); vertPos += 20.0f;

		std::string positionStr = "Position: "
			+ std::to_string(camera.position().x) + ", "
			+ std::to_string(camera.position().y) + ", "
			+ std::to_string(camera.position().z);
		nvgText(vg, 10.0f, vertPos, positionStr.c_str(), nullptr); vertPos += 20.0f;

		std::string yawStr = "Yaw: "
			+ std::to_string(Math::toDegrees(camera.yaw())) + "°"
			+ " Pitch: "
			+ std::to_string(Math::toDegrees(camera.pitch())) + "°";
		nvgText(vg, 10.0f, vertPos, yawStr.c_str(), nullptr); vertPos += 20.0f;

		std::string fovStr = "Field of View: "
			+ std::to_string(Math::toDegrees(camera.fieldOfView())) + "°";
		nvgText(vg, 10.0f, vertPos, fovStr.c_str(), nullptr); vertPos += 20.0f;

		std::string focusDistanceStr = "Focus distance: "
			+ std::to_string(camera.focusDistance());
		nvgText(vg, 10.0f, vertPos, focusDistanceStr.c_str(), nullptr); vertPos += 20.0f;

		nvgText(vg, 10.0f, vertPos, camera.isContinuousAutoFocus() ? "Autofocus ON" : "Autofocus OFF", nullptr);
		vertPos += 20.0f;

		std::string apertureStr = "Aperture: "
			+ std::to_string(camera.aperture());
		nvgText(vg, 10.0f, vertPos, apertureStr.c_str(), nullptr); vertPos += 20.0f;

		std::string bouncesStr = "Bounces: "
			+ std::to_string(m_bouncesLimit);
		nvgText(vg, 10.0f, vertPos, bouncesStr.c_str(), nullptr); vertPos += 20.0f;

		std::string debugStr = "Debug hit pos: "
			+ std::to_string(debugHitRecord.point.x) + ", "
			+ std::to_string(debugHitRecord.point.y) + ", "
			+ std::to_string(debugHitRecord.point.z);
		nvgText(vg, 10.0f, vertPos, debugStr.c_str(), nullptr); vertPos += 20.0f;

		vec3 focusPos = camera.getFocusPosition();
		std::string debugStr2 = "Debug focus pos: "
			+ std::to_string(focusPos.x) + ", "
			+ std::to_string(focusPos.y) + ", "
			+ std::to_string(focusPos.z);
		nvgText(vg, 10.0f, vertPos, debugStr2.c_str(), nullptr); vertPos += 20.0f;
	}

	nvgRestore(vg);
}
