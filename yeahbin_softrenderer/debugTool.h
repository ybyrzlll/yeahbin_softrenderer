#pragma once
//debug
#include<iostream>
using namespace std;
template<typename T>
void static showVector3(Vector3<T> v) {
	cout << v.x << "  " << v.y << "  " << v.z << endl;
}