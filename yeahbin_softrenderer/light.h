#pragma once
#include "mMath.h"
#include "camera.h"
//=====================================================================
// ��
//=====================================================================
struct Light {
	transform_t Transform;      // ����任��
	float **DepthTexture;            // �������zbuffer[y] Ϊ�� y��ָ��
	Camera Light_Camera;
};

struct PointLight : Light {
	Vector3f pos;
	PointLight(Vector3f p) :pos(p) {}
};

void Light_init(Light *light, int shadow_width, int shadow_height)
{
	int need = sizeof(void*) * (shadow_height)+shadow_width * shadow_height * 4;
	char *ptr = (char*)malloc(need);
	light->DepthTexture = (float**)ptr;
	ptr += sizeof(void*) * shadow_height;
	for (int j = 0; j < shadow_height; j++) {
		light->DepthTexture[j] = (float*)(ptr + shadow_width * 4 * j);
	}
}

void Light_clear(Light *light, int shadow_width, int shadow_height)
{
	for (int j = 0; j < shadow_height; j++) {
		for (int i = 0; i < shadow_width; i++)
			light->DepthTexture[j][i] = 1;
	}
}