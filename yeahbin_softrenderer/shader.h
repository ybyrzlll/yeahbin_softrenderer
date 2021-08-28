#pragma once
#include <algorithm>
#define shadow_bias 0.01f
#define texture_width 255
#define texture_height 255
using namespace mMath;
//
//template<typename T>
//static T max(T a, T b) { return a > b ? a : b; }

static Vector3f tex(IUINT32** texture, float u, float v) {
	int x, y;
	u = clamp(u, 0.0f, 1.0f);
	v = clamp(v, 0.0f, 1.0f);
	x = texture_width * u;
	y = texture_height * v;
	int r = texture[y][x] >> 16;
	int g = texture[y][x] & 0xff00 - 0xff;
	int b = texture[y][x] & 0xff;

	return Vector3f(r ,g, b);
}


class BaseShader {
public:
	virtual ~BaseShader() {};
	virtual Vector3f vertex(const Vector3f &vertex, const Vector3f &normal,
		const Vector3f &textureVals, const Vector3f &tangent,
		int index) = 0;
	virtual Vector3f fragment(float u, float v) = 0;
};

class TestShader : public BaseShader {
public:
	//Per object data
	Matrix4 MVP, MV, V, N;
	float ambientStrength = 1;
	Vector3f rgb{ 255,255,255 };
	Vector3f lightColor{ 0,0.1,1 };

	//Per vertex data
	Vector3f Color[3];
	Vector2f UV[3];

	//Per pixel data
	Vector3f interpColor;
	Vector2f interpUV;
	Vector3f ambient;


	Vector3f vertex(const Vector3f &vertex, const Vector3f &normal,
		const Vector3f &textureVals, const Vector3f &tangent,
		int index) override
	{
		//UV[index] = textureVals;
		////Vertex attributes
		//normals[index] = N.matMultDir(normal).normalized();
		//viewDir[index] = MV.matMultVec(vertex).normalized();
		//lightDir = V.matMultDir(light).normalized();

		return matrix_apply(MVP, vertex);
		//return vertex;
	}

	Vector3f fragment(float u, float v) override {
		//Interpolated stuff
		ambient = lightColor * ambientStrength;
		interpUV = UV[0] + (UV[1] - UV[0])* u + (UV[2] - UV[0]) * v;

		return ambient * rgb;
	}

};

class ShadowMapShader : public BaseShader {
public:
	//Per object data
	Matrix4 MVP, MVP_Inverse;
	Vector3f rgb{ 255,255,255 }, lightColor{ 1,1,1 };
	float ambientStrength = 1;
	Light light;

	//Per vertex data
	Vector3f pos[3];

	//Per pixel data
	Vector3f interpColor, interpPos, LightBit_pos;
	Vector3f ambient;


	Vector3f vertex(const Vector3f &vertex, const Vector3f &normal,
		const Vector3f &textureVals, const Vector3f &tangent,
		int index) override
	{
		pos[index] = matrix_apply(MVP, vertex);
		;		return pos[index];
	}

	Vector3f fragment(float u, float v) override {

		interpPos = pos[0] + (pos[1] - pos[0])* u + (pos[2] - pos[0]) * v;
		interpPos = matrix_apply(MVP_Inverse, interpPos);//当前相机的逆矩阵
		interpPos = matrix_apply(light.transform.MVP, interpPos);//光的MVP
		perspectiveDivide(&interpPos);//光的透视除法
		LightBit_pos = transform_viewport(&light.transform, &interpPos);//光的视口变换

		int Core = 3;//卷积范围
		float shadow = 0.0;
		float curDepth = interpPos.w;
		int x = LightBit_pos.x, y = LightBit_pos.y;
		if (0 < x && x < light.transform.w
			&& 0 < y && y< light.transform.h
			&& curDepth - shadow_bias > light.DepthTexture[y][x])
		{

			for (int i = -Core; i <= Core; i++) {
				for (int j = -Core; j <= Core; j++) {
					float pcfDepth = light.DepthTexture[y + i][x + j];
					shadow += curDepth - shadow_bias > pcfDepth ? 1.0 : 0.0;
				}
			}
			//shadow = 9.0;
		}

		shadow = shadow / ((Core * 2 + 1) * (Core * 2 + 1));

		ambient = lightColor * ambientStrength;

		return ambient * rgb * (1.0f - shadow);
	}

};

class TestTexShader : public BaseShader {
public:
	//Per object data
	Matrix4 MVP, MV, V, N;
	float ambientStrength = 1;
	Vector3f rgb{ 255,255,255 };
	Vector3f lightColor{ 0,0.1,1 };
	IUINT32 **texture;

	//Per vertex data
	Vector3f Color[3];
	Vector3f UV[3];

	//Per pixel data
	Vector3f interpColor;
	Vector3f interpUV;
	Vector3f ambient;


	Vector3f vertex(const Vector3f &vertex, const Vector3f &normal,
		const Vector3f &textureVals, const Vector3f &tangent,
		int index) override
	{
		UV[index] = textureVals;

		return matrix_apply(MVP, vertex);
	}

	Vector3f fragment(float u, float v) override {
		//Interpolated stuff
		ambient = lightColor * ambientStrength;
		interpUV = UV[0] + (UV[1] - UV[0])* u + (UV[2] - UV[0]) * v;

		return tex(texture, interpUV.x, interpUV.y);
	}

};

class PhongShader : public BaseShader {
public:
	//Per object data
	Matrix4 MVP, MV, V;
	float ambientStrength = 0.05, specularStrength = 1;
	Vector3f lightColorAmb{ 0.152, 0.152, 0.152 }, lightColorDiff{ 1,0.5,1 }, lightColorSpec{ 1,1,1 };
	Vector3f rgb{ 255,255,255 };
	PointLight *pointLight;

	//Per vertex data
	Vector3f normals[3], viewDir[3], lightDir[3];

	//Per pixel data
	Vector3f ambient, diffuse, spec, specular, interpNormal, interpViewDir, interpLightDir;
	Vector3f halfDir;
	float gloss = 5, diffStrength;

	Vector3f vertex(const Vector3f &vertex, const Vector3f &normal,
		const Vector3f &textureVals, const Vector3f &tangent,
		int index) override
	{
		//Vertex attributes
		normals[index] = normal;
		viewDir[index] = matrix_apply(MV, vertex).normalized();
		lightDir[index] = (pointLight->pos - vertex).normalized();

		return matrix_apply(MVP, vertex);
	}

	Vector3f fragment(float u, float v) override {
		//Interpolated stuff
		interpNormal = normals[0] + (normals[1] - normals[0])* u + (normals[2] - normals[0]) * v;
		interpViewDir = viewDir[0] + (viewDir[1] - viewDir[0])* u + (viewDir[2] - viewDir[0]) * v;
		interpLightDir = lightDir[0] + (lightDir[1] - lightDir[0])* u + (lightDir[2] - lightDir[0]) * v;

		//Ambient 
		ambient = lightColorAmb * ambientStrength;

		//Diffuse
		diffStrength = max(0.0f, (interpNormal.normalized()).dotProduct(interpLightDir.normalized()));
		diffuse = lightColorDiff * diffStrength;

		//Specular
		halfDir = (interpLightDir - interpViewDir).normalized();
		spec = pow(max(0.0f, (interpNormal.normalized()).dotProduct(halfDir)), gloss);
		specular = lightColorSpec * specularStrength * spec;

		//cout <<"ambient: "<< ambient.x << "  " << ambient.y << "  " << ambient.z << endl;
		//cout <<"diffuse: " << diffuse.x << "  " << diffuse.y << "  " << diffuse.z  << endl;
		//cout << "specular: " << specular.x << "  " << specular.y << "  " << specular.z << "  " << endl<<endl;

		return (ambient + diffuse + specular) * rgb;//   
	}

};

