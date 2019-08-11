#pragma once
#include <fstream>
#include <iostream>
using namespace std;

static void printDepthTexture(float ** photo, int w, int h) {
	int ScreenSizeX = w;
	int ScreenSizeY = h;

	ofstream fout("DepthTexture.ppm");
	fout << "P3\n" << ScreenSizeX << " " << ScreenSizeY << "\n255\n";
	cout << "¿ªÊ¼äÖÈ¾" << endl;
	for (int j = ScreenSizeY - 1; j >= 0; j--)
	{
		for (int i = 0; i < ScreenSizeX; i++)
		{
			int ir = 255 * (1 - photo[j][i]);
			int ig = 255 * (1 - photo[j][i]);
			int ib = 255 * (1 - photo[j][i]);
			//if(photo[j][i]!=1) cout << photo[j][i] << endl;
			fout << ir << " " << ig << " " << ib << "\n";
		}
	}
	cout << "äÖÈ¾Íê±Ï" << endl;
	fout.close();
}

static void printColorPhoto(IUINT32 ** photo, int w, int h) {
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