#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

//
// ���Ǳ�����ɫ
const int bcColor = 0x4C7047;

struct Color
{
	BYTE red;
	BYTE green;
	BYTE blue;
	int hex();
};

int Color::hex() {
	return (red << 16) + (green << 8) + blue;
}

void paint(int x,int y,COLORREF color) {
	HDC hdc = GetDC(NULL);
	SetPixel(hdc, x, y, color);
	ReleaseDC(NULL, hdc);
}

void getRect(LPVOID buffer,LPRECT lpRect)
{
	LONG width, height;
	LPBYTE data;
	DWORD sizeImage;
	LPBITMAPFILEHEADER lpFileHeader = (LPBITMAPFILEHEADER)buffer;

	// ͼ�����ݴ�С
	sizeImage = lpFileHeader->bfSize - lpFileHeader->bfOffBits;
	data = (LPBYTE)buffer + lpFileHeader->bfOffBits;

	printf("\n\n");
	LPBITMAPINFOHEADER lpInfoHeader = (LPBITMAPINFOHEADER)((LPBYTE)buffer + sizeof(BITMAPFILEHEADER));

	width = lpInfoHeader->biWidth;
	height = lpInfoHeader->biHeight;

	// һ����ɫ�����ݴ�С��һ����ɫ��width����ɫ���ܹ���height����ɫ
	int lineSize = sizeImage / height;

	lpRect->left = width;
	lpRect->right = 0;
	lpRect->bottom = height;
	lpRect->top = 0;

	// ��ʼ����
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int index = y*lineSize + x * 3;
			Color* color = (Color*)(data + index);
			COLORREF colorref = color->hex();
			if (colorref != bcColor)
			{
				// �����ɫ���Ǳ�����ɫ
				if (x > lpRect->right)
					lpRect->right = x;
				if (x < lpRect->left)
					lpRect->left = x;
				if (y > lpRect->top)
					lpRect->top = y;
				if (y < lpRect->bottom)
					lpRect->bottom = y;
			}
		}
	}
}

void writeFile(LPVOID buffer,RECT* rect)
{
	LONG width, height;
	LPBYTE data;
	DWORD sizeImage;

	// �ļ�ͷ
	LPBITMAPFILEHEADER lpFileHeader = (LPBITMAPFILEHEADER)buffer;
	// ͼ�����ݴ�С
	sizeImage = lpFileHeader->bfSize - lpFileHeader->bfOffBits;
	data = (LPBYTE)buffer + lpFileHeader->bfOffBits;

	// ��Ϣͷ
	LPBITMAPINFOHEADER lpInfoHeader = (LPBITMAPINFOHEADER)((LPBYTE)buffer + sizeof(BITMAPFILEHEADER));

	width = lpInfoHeader->biWidth;
	height = lpInfoHeader->biHeight;
	int lineSize = sizeImage / height;

	// �޸���Ϣ
	int newWidth = rect->right - rect->left;
	int newHeight = rect->top - rect->bottom;
	
	// ÿ����ɫ�����ݴ�С
	int newLineSize = newWidth * 3;
	if (newLineSize % 4 != 0) {
		int _tmp = newLineSize % 4;
		newLineSize += 4 - _tmp;
	}
	int newSizeImage = newHeight*newLineSize;
	// �ļ���С
	int newFileSize = newSizeImage + lpFileHeader->bfOffBits;
	BYTE* newBuffer = new BYTE[newSizeImage];

	lpFileHeader->bfSize = newFileSize;
	lpInfoHeader->biWidth = newWidth;
	lpInfoHeader->biHeight = newHeight;
	lpInfoHeader->biSizeImage = newSizeImage;
	// ��ʼ����
	for (int x = 0; x < newWidth; x++)
	{
		for (int y = 0; y < newHeight; y++)
		{
			int index = (y+rect->bottom)*lineSize + (x+rect->left) * 3;
			int newIndex = y*newLineSize + x * 3;
			newBuffer[newIndex] = ((LPBYTE)data)[index];
			newBuffer[newIndex + 1] = ((LPBYTE)data)[index + 1];
			newBuffer[newIndex + 2] = ((LPBYTE)data)[index + 2];
		}
	}
	HANDLE hFile = CreateFileA("D:/FFOutput/output.bmp"
		, GENERIC_WRITE | GENERIC_READ, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwWritenSize = 0;
	// ����ԭ�����ļ�ͷ��ֻ�������޸���
	BOOL bRet1 = WriteFile(hFile, buffer, lpFileHeader->bfOffBits, &dwWritenSize, NULL);
	BOOL bRet2 = WriteFile(hFile, newBuffer, newFileSize, &dwWritenSize, NULL);
	if (bRet1&&bRet2)
	{
		printf("�ļ�д��ɹ���\n");
	}
	else
	{
		printf("�ļ�д��ʧ��!\n");
	}
	CloseHandle(hFile);
}

void myReadFile(HANDLE hFile, LPVOID buffer, DWORD readSize)
{
	DWORD readedSize;
	ReadFile(hFile, buffer, readSize, &readedSize, NULL);
	RECT rect;
	getRect(buffer,&rect);
	writeFile(buffer, &rect);
}

void test()
{
	char* filePath = "D:/FFOutput/dinosaur (15).bmp";
	//filePath = "D:/Develop/tpvillage/cpputil/ImageTransform/ImageTransform/main.cpp";
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	DWORD fileSize;

	HANDLE hFile = CreateFileA(filePath
		, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		printf("���ļ�ʧ�ܣ�\n");
		return;
	}
	GetFileAttributesExA(filePath, GetFileExInfoStandard, &fileInfo);
	fileSize = fileInfo.nFileSizeLow;
	printf("file size = %d\n",fileSize);
	BYTE* buffer = new BYTE[fileSize];
	myReadFile(hFile, buffer, fileSize);
	CloseHandle(hFile);

}

int main()
{
	//test();
	system("pause");
	return 0;
}
