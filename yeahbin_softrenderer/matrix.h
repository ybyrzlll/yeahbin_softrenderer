#pragma once
#include <math.h>
#include "vector3D.h"
struct Matrix4//[列][行]
{
	float m[4][4];
	Matrix4() {};
	Matrix4 operator* (Matrix4 &rhs) {
		//Matrix dimensions
		Matrix4 results;
		int n = 4;
		for (int rows = 0; rows < n; ++rows) {
			for (int cols = 0; cols < n; ++cols) {
				float total = 0;
				//total value of multiplication with all submultiplications added together
				//sub represents submultiplications in the actual matrix multiplication
				//For a nxn matrix you have n submultiplications
				for (int sub = 1; sub < n + 1; ++sub) {
					int rowLhs = rows; //row ind left matrix
					int colLhs = sub - 1; //col ind left matrix
					int rowRhs = sub - 1; //row ind right matrix
					int colRhs = cols; //col ind right matrix

					total += this->m[rowLhs][colLhs] * rhs.m[rowRhs][colRhs];
				}
				results.m[rows][cols] = total;
			}
		}
		return results;
	}
};

// y = x * m 能够用于自身
static Vector3f  matrix_apply(const Matrix4 &m, const Vector3f &x) {
	float X = x.x, Y = x.y, Z = x.z, W = x.w;
	Vector3f y;
	y.x = X * m.m[0][0] + Y * m.m[1][0] + Z * m.m[2][0] + W * m.m[3][0];
	y.y = X * m.m[0][1] + Y * m.m[1][1] + Z * m.m[2][1] + W * m.m[3][1];
	y.z = X * m.m[0][2] + Y * m.m[1][2] + Z * m.m[2][2] + W * m.m[3][2];
	y.w = X * m.m[0][3] + Y * m.m[1][3] + Z * m.m[2][3] + W * m.m[3][3];
	return y;
}

static void matrix_set_identity(Matrix4 *m) {
	m->m[0][0] = m->m[1][1] = m->m[2][2] = m->m[3][3] = 1.0f;
	m->m[0][1] = m->m[0][2] = m->m[0][3] = 0.0f;
	m->m[1][0] = m->m[1][2] = m->m[1][3] = 0.0f;
	m->m[2][0] = m->m[2][1] = m->m[2][3] = 0.0f;
	m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
}

static void matrix_set_zero(Matrix4 *m) {
	m->m[0][0] = m->m[0][1] = m->m[0][2] = m->m[0][3] = 0.0f;
	m->m[1][0] = m->m[1][1] = m->m[1][2] = m->m[1][3] = 0.0f;
	m->m[2][0] = m->m[2][1] = m->m[2][2] = m->m[2][3] = 0.0f;
	m->m[3][0] = m->m[3][1] = m->m[3][2] = m->m[3][3] = 0.0f;
}

// 平移变换
static void matrix_set_translate(Matrix4 *m, float x, float y, float z) {
	matrix_set_identity(m);
	m->m[3][0] = x;
	m->m[3][1] = y;
	m->m[3][2] = z;
}

// 缩放变换
static void matrix_set_scale(Matrix4 *m, float x, float y, float z) {
	matrix_set_identity(m);
	m->m[0][0] = x;
	m->m[1][1] = y;
	m->m[2][2] = z;
}

//求逆矩阵-------------------------------------------------------------------

	//按第一行展开计算|A|
static float getA(Matrix4 *arcs, int n)
{
	if (n == 1)
	{
		return arcs->m[0][0];
	}
	float ans = 0;
	Matrix4 temp;
	int i, j, k;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n - 1; j++)
		{
			for (k = 0; k < n - 1; k++)
			{
				temp.m[j][k] = arcs->m[j + 1][(k >= i) ? k + 1 : k];

			}
		}
		float t = getA(&temp, n - 1);
		//cout << t << endl;
		if (i % 2 == 0)
		{
			ans += arcs->m[0][i] * t;
		}
		else
		{
			ans -= arcs->m[0][i] * t;
		}
	}
	return ans;
}

//求伴随矩阵 Adjugate Matrix
static void  getAStart(Matrix4 *arcs, int n, Matrix4 *ans)
{
	if (n == 1)
	{
		ans->m[0][0] = 1;
		return;
	}
	int i, j, k, t;
	Matrix4 temp;
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			//构造计算|A|的矩阵
			for (k = 0; k < n - 1; k++)
			{
				for (t = 0; t < n - 1; t++)
				{
					temp.m[k][t] = arcs->m[k >= i ? k + 1 : k][t >= j ? t + 1 : t];
				}
			}

			//计算|A|得到 Matrix of Cofactors
			ans->m[j][i] = getA(&temp, n - 1);  //此处顺便进行了转置
			if ((i + j) % 2 == 1)
			{
				ans->m[j][i] = -ans->m[j][i];
			}
		}
	}
}

//得到给定矩阵src的逆矩阵保存到des中。
static bool GetMatrixInverse(Matrix4 *src, int n, Matrix4 *des)
{
	float flag = getA(src, n);
	Matrix4 t;
	if (0 == flag)
	{
		//cout << "原矩阵行列式为0，无法求逆。请重新运行" << endl;
		return false;//如果算出矩阵的行列式为0，则不往下进行
	}
	else
	{
		getAStart(src, n, &t);
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				des->m[i][j] = t.m[i][j] / flag;
			}

		}
	}

	return true;
}

// 求逆矩阵
static void matrix_inverse(Matrix4 *src, Matrix4 *dest) {
	int n = 4;
	float flag = getA(src, n);
	Matrix4 t;
	getAStart(src, n, &t);
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			dest->m[i][j] = t.m[i][j] / flag;
		}

	}

}
//求逆矩阵-------------------------------------------------------------------

// 旋转矩阵
static void matrix_set_rotate(Matrix4 *m, float x, float y, float z, float theta) {
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta * 0.5f);
	Vector3f vec{ x, y, z, 1.0f };
	float w = qcos;
	vector_normalize(&vec);
	x = vec.x * qsin;
	y = vec.y * qsin;
	z = vec.z * qsin;
	m->m[0][0] = 1 - 2 * y * y - 2 * z * z;
	m->m[1][0] = 2 * x * y - 2 * w * z;
	m->m[2][0] = 2 * x * z + 2 * w * y;
	m->m[0][1] = 2 * x * y + 2 * w * z;
	m->m[1][1] = 1 - 2 * x * x - 2 * z * z;
	m->m[2][1] = 2 * y * z - 2 * w * x;
	m->m[0][2] = 2 * x * z - 2 * w * y;
	m->m[1][2] = 2 * y * z + 2 * w * x;
	m->m[2][2] = 1 - 2 * x * x - 2 * y * y;
	m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
	m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
	m->m[3][3] = 1.0f;
}