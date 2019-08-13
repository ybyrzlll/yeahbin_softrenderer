#pragma once
#include "mMath.h"
#include "light.h"
#include "shader.h"
#include "zbuffer.h"
//=====================================================================
// ��Ⱦ�豸
//=====================================================================
struct Device {
	Transform transform;      // ����任��
	Transform transform_inv;		// transform�������
	int width;                  // ���ڿ��
	int height;                 // ���ڸ߶�
	IUINT32 **framebuffer;      // ���ػ��棺framebuffer[y] ����� y��
	Zbuffer *zbuffer;            // ��Ȼ��棺zbuffer[y] Ϊ�� y��ָ��
	IUINT32 **texture;          // ����ͬ����ÿ������
	int tex_width;              // ������
	int tex_height;             // ����߶�
	float max_u;                // ��������ȣ�tex_width - 1
	float max_v;                // �������߶ȣ�tex_height - 1
	int render_state;           // ��Ⱦ״̬
	IUINT32 background;         // ������ɫ
	IUINT32 foreground;         // �߿���ɫ

	Light  *light;				//��
	BaseShader *shader;
};