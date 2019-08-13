#pragma once
#include "mMath.h"
#include "device.h"



//卷积核
static float percentage_closer(float ** src, const int * x, const int * y) {
	float res = 0.0f;
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			res += src[*x + i][*y + j];
		}
	}
	return res / 9.0f;
}

//滤波
static void Filter(float ** src, const int * width, const int * height) {
	for (int i = 1; i < *height - 1; i++)
		for (int j = 1; j < *width - 1; j++)
		{
			src[i][j] = percentage_closer(src, &i, &j);
		}
}

//百分比渐进过滤（PCF，percentage-closer filtering）
static float ShadowCalculation(const Device *device, const Vector3f *pos) {
	Vector3f cvv_pos;
	Vector3f world_pos;
	Vector3f Light_pos;
	Vector3f LightBit_pos;

	//transform_homogenizeInverse(&device->transform, &cvv_pos, pos);//逆视口变换//乘以w
	//world_pos = matrix_apply(device->transform_inv.MVP, cvv_pos);//逆矩阵(模视+投影变换)


	//Light_pos = matrix_apply(device->light->transform.MVP, world_pos);//光的模视+投影变换
	//perspectiveDivide(&Light_pos);//光的透视除法
	//transform_viewport(&device->light->transform, &LightBit_pos, &Light_pos);//光的视口变换

	int Core = 3;//卷积范围
	float shadow = 0.0;
	float curDepth = Light_pos.z / Light_pos.w;
	if (0 < (int)LightBit_pos.x && (int)LightBit_pos.x < device->light->transform.w
		&& 0 < (int)LightBit_pos.y && (int)LightBit_pos.y< device->light->transform.h
		&& curDepth - shadow_bias> device->light->DepthTexture[(int)LightBit_pos.y][(int)LightBit_pos.x])
	{

		for (int i = -Core; i <= Core; i++) {
			for (int j = -Core; j <= Core; j++) {
				float pcfDepth = device->light->DepthTexture[(int)LightBit_pos.y + i][(int)LightBit_pos.x + j];
				shadow += curDepth - shadow_bias > pcfDepth ? 1.0 : 0.0;
			}
		}
		//shadow = 9.0;
	}
	return shadow / ((Core * 2 + 1) * (Core * 2 + 1));
}