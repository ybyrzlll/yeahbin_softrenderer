#ifndef MMATH_H
#define MMATH_H
#include <math.h>
#include "vector3D.h"
#include "matrix.h"


template<typename T>
static T clamp(T x, T min, T max) { return (x < min) ? min : ((x > max) ? max : x); }

//=====================================================================
// ��ѧ�� D3D 
//=====================================================================


namespace mMath {
	typedef unsigned int IUINT32;

	// ���������  Look-At����
	/*void camera_set_LookAt(Matrix4 *m, const Vector3f& eye, const Vector3f &at, const Vector3f *up) {
		Vector3f xaxis, yaxis, zaxis;

		zaxis = at - eye;
		zaxis.normalized();
		vector_crossproduct(&xaxis, up, &zaxis);
		xaxis.normalized();
		vector_crossproduct(&yaxis, &zaxis, &xaxis);

		m->m[0][0] = xaxis.x;
		m->m[1][0] = xaxis.y;
		m->m[2][0] = xaxis.z;
		m->m[3][0] = -vector_dotproduct(&xaxis, &eye);

		m->m[0][1] = yaxis.x;
		m->m[1][1] = yaxis.y;
		m->m[2][1] = yaxis.z;
		m->m[3][1] = -vector_dotproduct(&yaxis, &eye);

		m->m[0][2] = zaxis.x;
		m->m[1][2] = zaxis.y;
		m->m[2][2] = zaxis.z;
		m->m[3][2] = -vector_dotproduct(&zaxis, &eye);

		m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
		m->m[3][3] = 1.0f;
	}*/

	// ���������  PHIGS����
	void camera_set_PHIGS(Matrix4 *m, const Vector3f *eye, const Vector3f *vpn, const Vector3f *up) {
		Vector3f xaxis, yaxis, zaxis;

		zaxis = *vpn;
		zaxis.normalized();
		vector_crossproduct(&xaxis, up, &zaxis);
		&xaxis.normalized();
		vector_crossproduct(&yaxis, &zaxis, &xaxis);

		m->m[0][0] = xaxis.x;
		m->m[1][0] = xaxis.y;
		m->m[2][0] = xaxis.z;
		m->m[3][0] = -vector_dotproduct(&xaxis, eye);

		m->m[0][1] = yaxis.x;
		m->m[1][1] = yaxis.y;
		m->m[2][1] = yaxis.z;
		m->m[3][1] = -vector_dotproduct(&yaxis, eye);

		m->m[0][2] = zaxis.x;
		m->m[1][2] = zaxis.y;
		m->m[2][2] = zaxis.z;
		m->m[3][2] = -vector_dotproduct(&zaxis, eye);

		m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
		m->m[3][3] = 1.0f;
	}

	// D3DXMatrixPerspectiveFovLH
	void matrix_set_perspective(Matrix4 *m, float fovy, float aspect, float zn, float zf) {
		float fax = 1.0f / (float)tan(fovy * 0.5f);
		matrix_set_zero(m);
		m->m[0][0] = (float)(fax / aspect);
		m->m[1][1] = (float)(fax);
		m->m[2][2] = zf / (zf - zn);
		m->m[3][2] = -zn * zf / (zf - zn);
		m->m[2][3] = 1;
	}


	//=====================================================================
	// ����任
	//=====================================================================
	struct Transform {
		Matrix4 world;         // ��������任  ���ڶ�������ϵ��Ϊ����localPos תworldPos������
		Matrix4 view;          // ��Ӱ������任
		Matrix4 projection;    // ͶӰ�任
		Matrix4 MVP;     // transform = world * view * projection
		float w, h;             // ��Ļ��С
	};


	// ������£����� transform = world * view * projection
	void transform_update(Transform *ts) {
		Matrix4 m;
		m = ts->world * ts->view;
		ts->MVP = m * ts->projection;
	}

	// ��ʼ����������Ļ����
	void transform_init(Transform *ts, int width, int height) {
		float aspect = (float)width / ((float)height);
		matrix_set_identity(&ts->world);
		matrix_set_identity(&ts->view);
		matrix_set_perspective(&ts->projection, 3.1415926f * 0.5f, aspect, 1.0f, 500.0f);
		ts->w = (float)width;
		ts->h = (float)height;
		transform_update(ts);
	}

	// ��ʸ�� x ���� project 
	void transform_apply(const Transform &ts, Vector3f &y, const Vector3f &x) {
		y = matrix_apply(ts.MVP, x);
	}

	// ����������ͬ cvv �ı߽�������׶�ü�  D3D
	int transform_check_cvv(const Vector3f& v) {
		float w = v.w;
		//printf("the number is: %f", w);
		int check = 0;
		if (v.z < 0.0f) check |= 1;
		if (v.z > w) check |= 2;
		if (v.x < -w) check |= 4;
		if (v.x > w) check |= 8;
		if (v.y < -w) check |= 16;
		if (v.y > w) check |= 32;
		return check;
	}

	//͸�ӳ���
	void perspectiveDivide(Vector3f *x) {
		//if (x->w < 1.000001&&x->w < 1.000001) return;//���w==1������������4����������
		float rhw = 1.0f / x->w;
		x->x = x->x * rhw;
		x->y = x->y * rhw;
		x->z = x->z * rhw;
	}

	//�ӿڱ任,�õ���Ļ����
	Vector3f transform_viewport(const Transform *ts, const Vector3f *x) {
		//�ӿڱ任
		Vector3f y;
		y.x = (x->x + 1.0f) * ts->w * 0.5f;
		y.y = (1.0f - x->y) * ts->h * 0.5f;
		y.z = x->z;
		y.w = x->w;
		return y;
	}

	//��͸�ӳ���
	void perspectiveDivide_Inverse(Vector3f *x) {
		x->x = x->x * x->w;
		x->y = x->y * x->w;
		x->z = x->z * x->w;
	}

	//���ӿڱ任 y = x...
	void transform_viewport_Inverse(const Transform *ts, Vector3f *y, const Vector3f *x) {
		y->x = (2 * x->x / ts->w - 1.0f);
		y->y = (1.0f - 2 * x->y / ts->h);
		y->z = x->z;
	}

	//=====================================================================
	// ���μ��㣺���㡢ɨ���ߡ���Ե�����Ρ���������
	//=====================================================================
	struct vertex_t {
		Vector3f pos;
		Vector2f tc;
		Vector3f normal;
		float rhw;
	};

	typedef struct { vertex_t v, v1, v2; } edge_t;
	typedef struct { float top, bottom; edge_t left, right; } trapezoid_t;
	typedef struct { vertex_t v, step; int x, y, w; } scanline_t; //��㣬�����㣨���ڱ���ʱ����ɫ����ı仯�� //��ʼx��y���ߵĳ���

	//����encode�˵�����ֱ����ƽ���ཻ�㣬����alphaֵ
	float  Intersect_Line_Plane(const Vector3f *p1, const Vector3f *p2,
		const int *a) {
		if (*a & 1) { return (-p1->z) / (p2->z - p1->z); }
		if (*a & 2) { return (1.0f - p1->z) / (p2->z - p1->z); }
		if (*a & 4) { return ((-1.0f - p1->x) / (p2->x - p1->x)); }
		if (*a & 8) { return (1.0f - p1->x) / (p2->x - p1->x); }
		if (*a & 16) { return (-1.0f - p1->y) / (p2->y - p1->y); }
		if (*a & 32) { return (1.0f - p1->y) / (p2->y - p1->y); }
	}

	//�����޳�
	bool backFaceCulling(Vector3f vpos, Vector3f fNormal, Vector3f eye) {
		float intensity = (vpos - eye).normalized().dotProduct(fNormal);
		return  intensity >= 0.0;
	}

	//�ò��������Χ�������ε����----ba���ca    c��ba�ұߴ���0
	float area_of_triangle(Vector3f &a, Vector3f &b, Vector3f &c) {
		return (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
	}
}
#endif