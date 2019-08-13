#include "zbuffer.h"
#include<iostream>

Zbuffer::Zbuffer(const int& w, const int& h)
{
	offset = 0.0f;
	this->height = h;
	this->width = w;
	val = new float*[h+1];
	for (int j = 0; j <= h; j++)
		val[j] = new float[w+1];
	for (int j = 0; j <= height; j++)
		for (int i = 0; i <= width; i++)
			val[j][i] = 0.0f;
}

bool Zbuffer::check(const float& depth, const int& j, const int& i)
{
	float res = 1.0f / depth + offset;
	if (res > val[j][i]) {
		val[j][i] = res;
		return true;
	}
	else
		return false;
}

void Zbuffer::clear()
{
	if (offset == offset + 1.0f) {
		offset = 0.0f;
		std::cout << "¸üÐÂÁËoffset" << std::endl;
		for (int j = 0; j <= height; j++)
			for (int i = 0; i <= width; i++)
				val[j][i] = 0.0f;
	}
	offset += 1.0f;
}

void Zbuffer::destroy()
{
	for (int i = 0; i < height; i++)
		delete []val[i];
	delete []val;
}
