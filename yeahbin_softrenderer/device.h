#pragma once
#include "mMath.h"
#include "light.h"
#include "shader.h"
//=====================================================================
// 渲染设备
//=====================================================================
typedef struct {
	transform_t transform;      // 坐标变换器
	transform_t transform_inv;		// transform的逆矩阵
	int width;                  // 窗口宽度
	int height;                 // 窗口高度
	IUINT32 **framebuffer;      // 像素缓存：framebuffer[y] 代表第 y行
	float **zbuffer;            // 深度缓存：zbuffer[y] 为第 y行指针
	IUINT32 **texture;          // 纹理：同样是每行索引
	int tex_width;              // 纹理宽度
	int tex_height;             // 纹理高度
	float max_u;                // 纹理最大宽度：tex_width - 1
	float max_v;                // 纹理最大高度：tex_height - 1
	int render_state;           // 渲染状态
	IUINT32 background;         // 背景颜色
	IUINT32 foreground;         // 线框颜色

	Light  *light;				//光
	BaseShader *shader;
}	device_t;