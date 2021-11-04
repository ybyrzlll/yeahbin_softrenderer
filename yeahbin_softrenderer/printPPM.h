#pragma once
#include <fstream>
#include <iostream>
#include "globalParam.h"
using namespace std;

namespace printPPM {
	typedef unsigned int IUINT32;

	void printDepthTexture(float ** photo, int w, int h) {
		int ScreenSizeX = w;
		int ScreenSizeY = h;

		ofstream fout("DepthTexture.ppm");
		fout << "P3\n" << ScreenSizeX << " " << ScreenSizeY << "\n255\n";
		cout << "¿ªÊ¼äÖÈ¾" << endl;
		for (int j = 0; j <= ScreenSizeY - 1; j++)
		{
			for (int i = 0; i < ScreenSizeX; i++)
			{
				float val = (1.0 - photo[j][i]/ Camera_FarDistane) * 255;// / Camera_FarDistane;
				if (photo[j][i] > 1.0) {
					//cout << val << endl;
				}
				else {
					val = 0;
				}
				fout << int(val) << " " << int(val) << " " << int(val) << "\n";
				//fout << ir << " " << ig << " " << ib << "\n";
			}
		}
		cout << "äÖÈ¾Íê±Ï" << endl;
		fout.close();
	}

	void printColorPhoto(IUINT32 ** photo, int w, int h) {
		int ScreenSizeX = w;
		int ScreenSizeY = h;

		ofstream fout("ColorPhoto.ppm");
		fout << "P3\n" << ScreenSizeX << " " << ScreenSizeY << "\n255\n";
		cout << "¿ªÊ¼äÖÈ¾" << endl;
		for (int j = ScreenSizeY - 1; j >= 0; j--)
		{
			for (int i = 0; i < ScreenSizeX; i++)
			{
				int ir = (int)(photo[j][i]) >> 16;
				int ig = ((int)(photo[j][i]) - (ir << 16)) >> 8;
				int ib = (int)(photo[j][i]) - (ir << 16) - (ig << 8);
				//cout << ir << " " << ig << "  " << ib << endl;

				fout << ir << " " << ig << " " << ib << "\n";
			}
		}
		cout << "äÖÈ¾Íê±Ï" << endl;
		fout.close();
	}
}
