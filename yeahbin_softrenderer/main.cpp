//=====================================================================
//=====================================================================

#include <iostream>
#include <assert.h>

#include "mMath.h"

#include "mPostProcess.h"
#include "mShadowMap.h"
#include "light.h"
#include "device.h"
#include "camera.h"
#include "shader.h"
#include "printPhoto.h"
#include "mesh.h"

#include <windows.h>
#include <tchar.h>

#define Camera_RotateSpeed 0.02

using namespace std;

#define RENDER_STATE_WIREFRAME      1		// 渲染线框
#define RENDER_STATE_COLOR          2		// 渲染颜色
#define RENDER_STATE_TEXTURE        4		// 渲染纹理
#define RENDER_STATE_DEPTHTEXTURE     8		// 渲染阴影深度纹理

// 设备初始化，fb为外部帧缓存，非 NULL 将引用外部帧缓存（每行 4字节对齐）
void device_init(device_t *device, int width, int height, void *fb) {
	int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
	char *ptr = (char*)malloc(need + 64);
	char *framebuf, *zbuf;
	int j;
	assert(ptr);
	device->framebuffer = (IUINT32**)ptr;
	device->zbuffer = (float**)(ptr + sizeof(void*) * height);
	ptr += sizeof(void*) * height * 2;
	device->texture = (IUINT32**)ptr;
	ptr += sizeof(void*) * 1024;
	framebuf = (char*)ptr;
	zbuf = (char*)ptr + width * height * 4;
	ptr += width * height * 8;
	if (fb != NULL) framebuf = (char*)fb;
	for (j = 0; j < height; j++) {
		device->framebuffer[j] = (IUINT32*)(framebuf + width * 4 * j);
		device->zbuffer[j] = (float*)(zbuf + width * 4 * j);
	}
	device->texture[0] = (IUINT32*)ptr;
	device->texture[1] = (IUINT32*)(ptr + 16);
	memset(device->texture[0], 0, 64);
	device->tex_width = 2;
	device->tex_height = 2;
	device->max_u = 1.0f;
	device->max_v = 1.0f;
	device->width = width;
	device->height = height;
	device->background = 0xc0c0c0;
	device->foreground = 0;
	transform_init(&device->transform, width, height);
	device->render_state = RENDER_STATE_WIREFRAME;
}

// 删除设备
void device_destroy(device_t *device) {
	if (device->framebuffer)
		free(device->framebuffer);
	device->framebuffer = NULL;
	device->zbuffer = NULL;
	device->texture = NULL;
}

// 设置当前纹理
void device_set_texture(device_t *device, void *bits, long pitch, int w, int h) {
	char *ptr = (char*)bits;
	int j;
	assert(w <= 1024 && h <= 1024);
	for (j = 0; j < h; ptr += pitch, j++) 	// 重新计算每行纹理的指针
		device->texture[j] = (IUINT32*)ptr;
	device->tex_width = w;
	device->tex_height = h;
	device->max_u = (float)(w - 1);
	device->max_v = (float)(h - 1);
}

// 清空 framebuffer 和 zbuffer
void device_clear(device_t *device, int mode) {
	int y, x, height = device->height;
	for (y = 0; y < device->height; y++) {
		IUINT32 *dst = device->framebuffer[y];
		IUINT32 cc = (height - 1 - y) * 230 / (height - 1);//？
		cc = (cc << 16) | (cc << 8) | cc;//？
		if (mode == 0) cc = device->background;
		for (x = device->width; x > 0; dst++, x--) dst[0] = cc;
	}
	for (y = 0; y < device->height; y++) {
		float *dst = device->zbuffer[y];
		for (x = device->width; x > 0; dst++, x--) {
			dst[0] = 1.0f;
		}
	}
}

// 画点
void device_pixel(device_t *device, int x, int y, IUINT32 color) {
	if (((IUINT32)x) < (IUINT32)device->width && ((IUINT32)y) < (IUINT32)device->height) {
		device->framebuffer[y][x] = color;
	}
}

// 绘制线段   DDA
void device_draw_line(device_t *device, int x1, int y1, int x2, int y2, IUINT32 c) {
	int x, y, rem = 0;
	if (x1 == x2 && y1 == y2) {
		device_pixel(device, x1, y1, c);
	}
	else if (x1 == x2) {
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) device_pixel(device, x1, y, c);
		device_pixel(device, x2, y2, c);
	}
	else if (y1 == y2) {
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) device_pixel(device, x, y1, c);
		device_pixel(device, x2, y2, c);
	}
	else {
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		if (dx >= dy) {
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x <= x2; x++) {
				device_pixel(device, x, y, c);
				rem += dy;
				if (rem >= dx) {
					rem -= dx;
					y += (y2 >= y1) ? 1 : -1;
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}
		else {
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y <= y2; y++) {
				device_pixel(device, x, y, c);
				rem += dx;
				if (rem >= dy) {
					rem -= dy;
					x += (x2 >= x1) ? 1 : -1;
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}
	}
}

// 根据坐标读取纹理
IUINT32 device_texture_read(const device_t *device, float u, float v) {
	int x, y;
	u = u * device->max_u;
	v = v * device->max_v;
	x = (int)(u + 0.5f);
	y = (int)(v + 0.5f);
	x = clamp(x, 0, device->tex_width - 1);
	y = clamp(y, 0, device->tex_height - 1);
	return device->texture[y][x];
}

//=====================================================================
// 渲染实现
//=====================================================================

//光栅化----重心坐标权重插值
void rasterize(device_t *device, Vector3f &c1,
	Vector3f &c2, Vector3f &c3) {

	float area = area_of_triangle(c1, c2, c3);
	if (area <= 0.0001 && area >= -0.0001) return;//在同一直线上
	area = 1 / area;

	int render_state = device->render_state;

	Vector3f rgbVals{ 255,255,255 };
	Vector3f perCor{ 1 / c1.w, 1 / c2.w, 1 / c3.w };
	float SumCor;

	Vector3f zVals({ c1.z, c2.z, c3.z });

	//find triangle boding box
	int xMax, xMin, yMax, yMin;
	xMax = max(max(c1.x, c2.x), c3.x);
	xMin = min(min(c1.x, c2.x), c3.x);
	yMax = max(max(c1.y, c2.y), c3.y);
	yMin = min(min(c1.y, c2.y), c3.y);
	xMax = min(xMax, device->width);
	xMin = max(xMin, 0);
	yMax = min(yMax, device->height);
	yMin = max(yMin, 0);

	//step   X/Y的增量=面积的增量
	float Y23 = c3.x - c2.x, X23 = c2.y - c3.y;
	float Y31 = c1.x - c3.x, X31 = c3.y - c1.y;
	float Y12 = c2.x - c1.x, X12 = c1.y - c2.y;

	Vector3f e, e_row;
	Vector3f point{ (float)xMin, (float)yMin, 0 };
	e_row.x = area_of_triangle(c2, c3, point);
	e_row.y = area_of_triangle(c3, c1, point);
	e_row.z = area_of_triangle(c1, c2, point);

	for (int y = yMin; y <= yMax; y++) {
		e = e_row;

		for (int x = xMin; x <= xMax; x++) {
			//if (abs(e.x) + abs(e.y) + abs(e.z) <=abs(1/area)+1)使用面积判断是否在三角形内
			if (e.x >= 0 && e.y >= 0 && e.z >= 0)//该点在顺序三边的同一侧则在三角形内
			{
				//透视矫正 perspective correct
				Vector3f temp = e * perCor;//e/area即为该三角形占整个三角形的比例
				SumCor = 1 / (temp.x + temp.y + temp.z);
				Vector3f lambda = temp * SumCor;
				float curDepth = lambda.dotProduct(zVals);
				if (curDepth < device->zbuffer[y][x]) {

					device->zbuffer[y][x] = curDepth;

					if (render_state & RENDER_STATE_DEPTHTEXTURE)
					{
						device->light->DepthTexture[y][x] = curDepth;
						//cout << curDepth << endl;
					}
					else if (render_state & RENDER_STATE_COLOR) {
						rgbVals = device->shader->fragment(lambda.y, lambda.z);
						rgbVals.x = clamp(rgbVals.x, 0.0f, 255.0f);
						rgbVals.y = clamp(rgbVals.y, 0.0f, 255.0f);
						rgbVals.z = clamp(rgbVals.z, 0.0f, 255.0f);
						float sum = ((int)rgbVals.x << 16) + ((int)rgbVals.y << 8) + rgbVals.z;
						device->framebuffer[y][x] = sum;
					}
				}
			}
			e.x += X23;
			e.y += X31;
			e.z += X12;
		}
		e_row.x += Y23;
		e_row.y += Y31;
		e_row.z += Y12;
	}
}


void device_draw_primitive2(device_t *device, const  Vector3f *vertices) {
	Vector3f p1 = vertices[0], p2 = vertices[1], p3 = vertices[2];
	int render_state = device->render_state;

	// 归一化
	perspectiveDivide(&p1);
	perspectiveDivide(&p2);
	perspectiveDivide(&p3);
	//视口变换
	p1 = transform_viewport(&device->transform, &p1);
	p2 = transform_viewport(&device->transform, &p2);
	p3 = transform_viewport(&device->transform, &p3);

	// 纹理或者色彩绘制
	if (render_state & (RENDER_STATE_TEXTURE | RENDER_STATE_COLOR | RENDER_STATE_DEPTHTEXTURE)) {

		//重心权值--光栅化
		rasterize(device, p1, p2, p3);

	}

	if (render_state & RENDER_STATE_WIREFRAME) {		// 线框绘制
		device_draw_line(device, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, device->foreground);
		device_draw_line(device, (int)p1.x, (int)p1.y, (int)p3.x, (int)p3.y, device->foreground);
		device_draw_line(device, (int)p3.x, (int)p3.y, (int)p2.x, (int)p2.y, device->foreground);
	}
}

//Cohen-Sutherland 裁剪算法
void Crop_Cohen_Sutherland(device_t *device, Vector3f *vertices) {
	int s1 = transform_check_cvv(vertices[0]);
	int s2 = transform_check_cvv(vertices[1]);
	int s3 = transform_check_cvv(vertices[2]);

	if (s1 == 0 || s2 == 0 || s3 == 0)//片元裁剪
	//if (s1 == 0 && s2 == 0 && s3 == 0)//顶点裁剪   
	{
		device_draw_primitive2(device, vertices);
		return;
	}
	if (s1 && s2 && s3 != 0) return;//舍弃
	if (s1 != 0 && s2 != 0 && s3 != 0) return;//3外，暂时舍弃

	//裁剪
	Vector3f  c1 = vertices[0], c2 = vertices[0], c3 = vertices[0];
	//归一化方便计算交点
	perspectiveDivide(&c1);
	perspectiveDivide(&c2);
	perspectiveDivide(&c3);

	//排序：内》外
	int temp;
	Vector3f v_temp;
	if (s1 != 0 && s2 == 0) { v_temp = c1; c1 = c2; c2 = v_temp; temp = s1; s1 = s2; s2 = temp; }
	if (s1 != 0 && s3 == 0) { v_temp = c1; c1 = c3; c3 = v_temp; temp = s1; s1 = s3; s3 = temp; }
	if (s2 != 0 && s3 == 0) { v_temp = c2; c2 = c3; c3 = v_temp; temp = s2; s2 = s3; s3 = temp; }

	if (s2 != 0) {//1内2外  也可以用2内1外递归?
		//vertex_t c4, c5, c6, c7;
		//float alpha_4 = Intersect_Line_Plane(&c1.pos, &c3.pos, &s3);
		////alpha_4 = clamp(alpha_4, 0, 1);
		//vertex_interp(&c4, &c1, &c3, alpha_4);

		//float alpha_5 = Intersect_Line_Plane(&c2.pos, &c3.pos, &s3);
		////alpha_5 = clamp(alpha_5, 0, 1);
		//vertex_interp(&c5, &c2, &c3, alpha_5);

		//float alpha_6 = Intersect_Line_Plane(&c2.pos, &c3.pos, &s2);
		////alpha_6 = clamp(alpha_6, 0, 1);
		//vertex_interp(&c6, &c2, &c3, alpha_6);

		//float alpha_7 = Intersect_Line_Plane(&c1.pos, &c2.pos, &s2);
		////alpha_7 = clamp(alpha_7, 0, 1);
		//vertex_interp(&c7, &c1, &c2, alpha_7);
		//
		//Crop_Cohen_Sutherland(device, &c1, &c4, &c5);
		//Crop_Cohen_Sutherland(device, &c1, &c5, &c6);
		//Crop_Cohen_Sutherland(device, &c1, &c6, &c7);
	}
	else
	{//2内1外
		//vertex_t c4, c5;
		//float alpha_4 = Intersect_Line_Plane(&c1, &c3, &s3);
		//vertex_interp(&c4, &c1, &c3, alpha_4);

		//float alpha_5 = Intersect_Line_Plane(&c2, &c3, &s3);//
		//vertex_interp(&c5, &c2, &c3, alpha_5);

		////新生成的c4, c5可能不在cvv内，需要继续裁剪
		//Crop_Cohen_Sutherland(device, &c1, &c2, &c4);
		//Crop_Cohen_Sutherland(device, &c2, &c4, &c5);
	}
}




//=====================================================================
// Win32 窗口及图形绘制：为 device 提供一个 DibSection 的 FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// 当前键盘按下状态
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// 老的 BITMAP
unsigned char *screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR *title);	// 屏幕初始化
int screen_close(void);								// 关闭屏幕
void screen_dispatch(void);							// 处理消息
void screen_update(void);							// 显示 FrameBuffer

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// 初始化窗口并设置标题
int screen_init(int w, int h, const TCHAR *title) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

int screen_close(void) {
	if (screen_dc) {
		if (screen_ob) {
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) {
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}
	if (screen_handle) {
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}

static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void screen_dispatch(void) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);//将一幅位图从screen_dc设备场景复制到另一个hDC
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}


//=====================================================================
// 主程序
//=====================================================================
//平面
#define Plane_Size 3.5
vertex_t mesh_plane[4] = {
	{ { -Plane_Size, -2,  -Plane_Size, 1 }, { 0, 0 }, { 1.0f, 1.0f, 1.0f }, 1 },
	{ { -Plane_Size, -2,  Plane_Size, 1 }, { 0, 1 }, { 1.0f, 1.0f, 1.0f }, 1 },
	{ {  Plane_Size, -2,  Plane_Size, 1 }, { 1, 1 }, { 1.0f, 0.2f, 1.0f }, 1 },
	{ {  Plane_Size, -2,  -Plane_Size, 1 }, { 1, 0 }, { 1.0f, 1.0f, 1.0f }, 1 }
};

//void draw_plane(device_t *device) {
//	vertex_t p1 = mesh_plane[0], p2 = mesh_plane[1], p3 = mesh_plane[2], p4 = mesh_plane[3];
//	p1.tc.u = 0, p1.tc.v = 0, p2.tc.u = 0, p2.tc.v = 1;
//	p3.tc.u = 1, p3.tc.v = 1, p4.tc.u = 1, p4.tc.v = 0;
//	device_draw_primitive(device, &p1, &p2, &p3);
//	device_draw_primitive(device, &p3, &p4, &p1);
//}

//测试裁剪
//void draw_Triangle(device_t *device) {
//	vertex_t p1 = mesh_plane[0], p2 = mesh_plane[1], p3 = mesh_plane[2];
//	p1.tc.u = 0, p1.tc.v = 0, p2.tc.u = 0, p2.tc.v = 1;
//	p3.tc.u = 1, p3.tc.v = 1;
//	device_draw_primitive(device, &p1, &p2, &p3);
//}

//立方体
vertex_t mesh[8] = {
	{ { -1, -1,  1, 1 }, { 0, 0 }, { 1.0f, 0.2f, 0.2f }, 1 },
	{ {  1, -1,  1, 1 }, { 0, 1 }, { 0.2f, 1.0f, 0.2f }, 1 },
	{ {  1,  1,  1, 1 }, { 1, 1 }, { 0.2f, 0.2f, 1.0f }, 1 },
	{ { -1,  1,  1, 1 }, { 1, 0 }, { 1.0f, 0.2f, 1.0f }, 1 },
	{ { -1, -1, -1, 1 }, { 0, 0 }, { 1.0f, 1.0f, 0.2f }, 1 },
	{ {  1, -1, -1, 1 }, { 0, 1 }, { 0.2f, 1.0f, 1.0f }, 1 },
	{ {  1,  1, -1, 1 }, { 1, 1 }, { 1.0f, 0.3f, 0.3f }, 1 },
	{ { -1,  1, -1, 1 }, { 1, 0 }, { 0.2f, 1.0f, 0.3f }, 1 },
};


void packDataIntoTris(Vector3i &index, Vector3f *primitive, std::vector<Vector3f> &vals) {
	for (int i = 0; i < 3; ++i) {
		primitive[i] = vals[index.data[i]];
	}
}

void draw_mesh(device_t *device, Mesh *mesh) {
	std::vector<Vector3i> * vIndices = &mesh->vertexIndices;
	std::vector<Vector3i> * tIndices = &mesh->textureIndices;
	std::vector<Vector3i> * nIndices = &mesh->normalsIndices;
	std::vector<Vector3f> * fNormals = &mesh->fNormals;

	std::vector<Vector3f> * vertices = &mesh->vertices;
	std::vector<Vector3f> * texels = &mesh->texels;
	std::vector<Vector3f> * normals = &mesh->normals;
	std::vector<Vector3f> * tangents = &mesh->tangents;
	int numFaces = mesh->numFaces;

	for (int j = 0; j < numFaces; ++j) {

		//Arrays used to group vertices together into triangles
		Vector3f trianglePrimitive[3], normalPrim[3], uvPrim[3],
			tangentPrim[3];

		//Current vertex, normals and texture data indices
		Vector3i f = (*vIndices)[j];
		Vector3i n = (*nIndices)[j];
		Vector3i u = (*tIndices)[j];

		//Pack vertex, normal and UV data into triangles
		packDataIntoTris(f, trianglePrimitive, *vertices);
		packDataIntoTris(n, normalPrim, *normals);
		packDataIntoTris(u, uvPrim, *texels);
		//packDataIntoTris(f, tangentPrim, *tangents);

		Vector3f lightDir[3];

		for (int i = 0; i < 3; ++i) {
			//顶点着色
			trianglePrimitive[i] = device->shader->vertex(trianglePrimitive[i], normalPrim[i],
				uvPrim[i], tangentPrim[i], i);
			/*cout << normalPrim[i].x<<"  " << normalPrim[i].y
				<< "  " << normalPrim[i].z << "  " << normalPrim[i].w << endl;*/
		}

		Crop_Cohen_Sutherland(device, trianglePrimitive);
	}
}

void camera_at_zero(device_t *device, Camera camera) {

	//camera_set_LookAt(&device->transform.view, &camera.pos, &camera.at, &camera.up);
	camera_set_PHIGS(&device->transform.view, &camera.pos, &camera.vpn, &camera.up);
	transform_update(&device->transform);
	//计算矩阵的逆
	matrix_inverse(&device->transform.MVP, &device->transform_inv.MVP);
	matrix_inverse(&device->transform.world, &device->transform_inv.world);
}

void init_texture(device_t *device) {
	static IUINT32 texture[256][256];
	int i, j;
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			int x = i / 32, y = j / 32;
			//texture[j][i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;//skyblue
			texture[j][i] = ((x + y) & 1) ? 0xffffff : 0xffb6c1;//pink
		}
	}
	device_set_texture(device, texture, 256 * 4, 256, 256);
}

int main(void)
{

	Mesh cuboid, plane;
	buildMeshFromFile(cuboid, "Mesh/cuboid.obj");
	buildMeshFromFile(plane, "Mesh/plane.obj");

	Camera camera;
	int states[] = {
		RENDER_STATE_WIREFRAME,
		RENDER_STATE_COLOR,
		RENDER_STATE_TEXTURE,
		RENDER_STATE_DEPTHTEXTURE };
	int indicator = 0;
	int kbhit = 0;
	float thetaY, thetaZ;
	Vector3f pos;

	//===============================shadow map===============================
	//预计算深度纹理，使用另一渲染设备device_shadowmap
	const int shadowmap_width = 1200,
		shadowmap_height = 900;
	device_t device_shadowmap;
	device_init(&device_shadowmap, shadowmap_width, shadowmap_height, screen_fb);
	device_shadowmap.render_state = RENDER_STATE_DEPTHTEXTURE;
	PointLight pointLight(Vector3f(7, 7, -7, 1));
	Light_init(&pointLight, shadowmap_width, shadowmap_height);
	device_shadowmap.light = &pointLight;//将光添加到设备

	//设置光线投影
	camera.pos = pointLight.pos;
	camera.vpn = { -7, -7, 7, 1 };
	camera.up = { 0, 1, 0, 1 };

	camera_at_zero(&device_shadowmap, camera);//根据摄像机计算矩阵
	pointLight.Transform = device_shadowmap.transform;//记录 转换到光源空间的变换矩阵+wh
	device_clear(&device_shadowmap, 1);//初始化两个buffer
	Light_clear(&pointLight, shadowmap_width, shadowmap_height);

	//着色器装配
	TestShader shader;
	shader.MVP = device_shadowmap.transform.MVP;
	device_shadowmap.shader = &shader;

	//渲染图像入深度纹理
	draw_mesh(&device_shadowmap, &cuboid);
	draw_mesh(&device_shadowmap, &plane);

	device_destroy(&device_shadowmap);
	//printDepthTexture(pointLight.DepthTexture, shadowmap_width, shadowmap_height);
	//===============================shadow map===============================

	int window_width = 800, window_height = 600;
	//初始化窗口并设置标题
	char text[] = _T("YeahBin (software render ) - ");
	TCHAR *title = text;

	if (screen_init(window_width, window_height, title))//产生了外部缓存
		return -1;

	//初始化渲染设备
	device_t device;
	device_init(&device, window_width, window_height, screen_fb);

	//device.light = &pointLight;//将光添加到该设备


	//设置主相机
	camera.pos = { 5, 2.5, 5, 1 };
	camera.vpn = { -5, -2.5, -5, 1 };
	vector_normalize(&camera.vpn);
	camera.up = { 0, 1, 0, 1 };
	camera_at_zero(&device, camera);
	pos = camera.pos;
	float Camera_Speed = 0.1f;//摄像机运动速度
	vector_zoom(&camera.vpn, Camera_Speed);

	init_texture(&device);
	//printColorPhoto(device.texture, 256, 256);
	device.render_state = RENDER_STATE_COLOR;

	//BaseShader *shaderSwtich;

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch();
		device_clear(&device, 1);
		camera_at_zero(&device, camera);

		Vector3f camera_z;
		vector_crossproduct(&camera_z, &camera.vpn, &camera.up);
		vector_normalize(&camera_z);
		vector_zoom(&camera_z, Camera_Speed);

		if (screen_keys[0x41]) vector_add(&camera.pos, &camera.pos, &camera_z);//a
		if (screen_keys[0x44]) vector_sub(&camera.pos, &camera.pos, &camera_z);//d

		//camera.pos.x += 0.1f;//d
		if (screen_keys[0x57]) vector_add(&camera.pos, &camera.pos, &camera.vpn);//w
		if (screen_keys[0x53]) vector_sub(&camera.pos, &camera.pos, &camera.vpn);//s

		thetaY = thetaZ = 0;
		if (screen_keys[VK_LEFT]) thetaY = -Camera_RotateSpeed;
		if (screen_keys[VK_RIGHT]) thetaY = Camera_RotateSpeed;
		if (screen_keys[VK_UP]) thetaZ = -Camera_RotateSpeed;
		if (screen_keys[VK_DOWN]) thetaZ = Camera_RotateSpeed;

		//乘以四元数旋转矩阵
		Matrix4 m1, m2;
		matrix_set_rotate(&m1, 0, 1, 0, thetaY);
		if (camera.vpn.x > 0) matrix_set_rotate(&m2, 0, 0, -1, thetaZ);
		else  matrix_set_rotate(&m2, 0, 0, 1, thetaZ);
		camera.vpn = matrix_apply(m1, camera.vpn);
		camera.vpn = matrix_apply(m2, camera.vpn);

		if (screen_keys[VK_SPACE]) {
			if (kbhit == 0) {
				kbhit = 1;
				if (++indicator >= 2) indicator = 0;
				device.render_state = states[indicator];
			}
		}
		else {
			kbhit = 0;
		}

		//着色器--普通
		PhongShader ordinaryShader;
		ordinaryShader.MVP = device.transform.MVP;

		//着色器--贴纹理
		TestTexShader texShader;
		texShader.MVP = device.transform.MVP;
		texShader.texture = device.texture;

		//着色器--shawdowmap阴影
		ShadowMapShader shadowMapShader;
		shadowMapShader.MVP = device.transform.MVP;
		shadowMapShader.MVP_Inverse = device.transform_inv.MVP;
		shadowMapShader.light = pointLight;

		//着色器--phong
		PhongShader phongShader;
		phongShader.MVP = device.transform.MVP;
		phongShader.MV = device.transform.world * device.transform.view;
		phongShader.pointLight = &pointLight;

		//device.shader = &texShader;
		//device.shader = &shadowMapShader;
		//device.shader = &ordinaryShader;
		device.shader = &phongShader;

		draw_mesh(&device, &plane);



		draw_mesh(&device, &cuboid);

		screen_update();
		Sleep(1);
	}
	return 0;
}

