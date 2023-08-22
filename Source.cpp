#undef UNICODE
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRTusing 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "WinApiHelper.h"
#include <iostream>
#include <filesystem>

int main() {
	setlocale(0, "ru");



	char aboba[MAX_PATH] = { '\0' };
	SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, aboba, 0);
	wcout << aboba;

}