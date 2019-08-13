#pragma once
struct Zbuffer {
	float** val;
	float offset;//相当于第几帧
	int width, height;

	Zbuffer(const int& w, const int& h);
	bool check(const float& depth, const int& j, const int& i);
	void clear();//不是每帧都遍历一遍整个val
	void destroy();
};