/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

#include <et/core/interpolationvalue.h>
#include <et/camera/cameracontroller.h>
#include <et/input/gestures.h>

namespace et
{
	class CameraMovingController : public CameraController
	{
	public:
		ET_DECLARE_POINTER(CameraMovingController);
		
	public:
		CameraMovingController(Camera&, bool autoConnectToEvents);
		
		void synchronize(const Camera&) override;
		
		void setMovementSpeed(const vec3&);
		void setIntepolationRate(float);

		void startUpdates() override;
		void cancelUpdates() override;
		
	private:
		void update(float) override;
		
		void onKeyPressed(size_t) override;
		void onKeyReleased(size_t) override;
		void onPointerPressed(PointerInputInfo) override;
		void onPointerMoved(PointerInputInfo) override;
		void onPointerReleased(PointerInputInfo) override;
		void onPointerCancelled(PointerInputInfo) override;
		
		void validateCameraAngles(vec2&);
		
	private:
		GesturesRecognizer _gestures;
		std::map<size_t, int> _pressedKeys;
		InterpolationValue<vec3> _positionAnimator;
		InterpolationValue<vec2> _directionAnimator;
		vec3 _movementSpeed = vec3(1.0f);
	};
}
