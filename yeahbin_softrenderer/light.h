#pragma once
#include "camera.h"
using namespace mMath;
//=====================================================================
// ��
//=====================================================================
class Light {
public:
	Transform transform;      // ����任��
	float **DepthTexture;            // �������zbuffer[y] Ϊ�� y��ָ��
	Camera Light_Camera;

	void DepthTexture_init(int width, int height) {
		DepthTexture = new float*[height + 1];
		for (int j = 0; j <= height; j++)
			DepthTexture[j] = new float[width];
	};
	void Light_clear(int shadow_width, int shadow_height) {
		for (int j = 0; j < shadow_height; j++) {
			for (int i = 0; i < shadow_width; i++)
				DepthTexture[j][i] = 1;
		}
	};
};

class PointLight : public Light {
public:
	Vector3f pos;
	PointLight(Vector3f p) :pos(p) {}
};