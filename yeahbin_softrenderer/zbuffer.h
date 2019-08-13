#pragma once
struct Zbuffer {
	float** val;
	float offset;//�൱�ڵڼ�֡
	int width, height;

	Zbuffer(const int& w, const int& h);
	bool check(const float& depth, const int& j, const int& i);
	void clear();//����ÿ֡������һ������val
	void destroy();
};