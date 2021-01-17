#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include <sstream>
using namespace std;

//
// ���Ǳ�����ɫ
const int bcColor = 0x4C7047;

struct Color
{
	BYTE red;
	BYTE green;
	BYTE blue;
	COLORREF hex();
};

COLORREF Color::hex() {
	return (red << 16) + (green << 8) + blue;
}

/**
* @brief ��ȡ�ļ�����
*		�������ͷ��ڴ�
* @param fileName �ļ���
* @param lpReaddedSize ʵ�ʶ�ȡ���Ĵ�С
* @return ��ȡ�������ݵĻ����������������ͷ��ڴ棩
*/
LPVOID readFormFile(LPCSTR fileName, LPDWORD lpReadedSize)
{
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;		// �ļ���Ϣ
	DWORD fileSize;							// �ļ���С

	HANDLE hFile = CreateFileA(fileName
		, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//printf("���ļ�ʧ�ܣ�\n");
		return NULL;
	}
	// ��ȡ�ļ���С
	GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileInfo);
	fileSize = fileInfo.nFileSizeLow;
	
	LPBYTE readBuffer = new BYTE[fileSize];
	ReadFile(hFile, readBuffer, fileSize, lpReadedSize, NULL);
	CloseHandle(hFile);
	return readBuffer;
}

/**
* @brief ��ָ�����������ڴ�д���ļ���
* @param outputFileName ������ļ���
* @param buffer ������
* @param size Ҫд��Ĵ�С
* @return ʵ��д��Ĵ�С
*/
DWORD writeToFile(LPCSTR outputFileName,LPVOID buffer,DWORD size)
{
	DWORD dwWriteSize;
	HANDLE hFile = CreateFileA(outputFileName
		, GENERIC_WRITE | GENERIC_READ, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	BOOL bRet = WriteFile(hFile, buffer, size, &dwWriteSize, NULL);
	if (bRet)
	{
		//printf("�ļ�д��ɹ���\n");
	}
	else
	{
		printf("�ļ�д��ʧ��!�ļ���С:%d\n",size);
	}
	CloseHandle(hFile);
	return size;
}

/**
* @brief ��ȡͼƬҪ�ü�������
* @param buffer ͼƬ���ݵĻ�����
* @param lpRect �ü������ָ��
*/
void getRect(LPVOID buffer,LPRECT lpRect)
{
	DWORD	width;			// ԭʼλͼ���
	DWORD	height;			// ԭʼλͼ�߶�
	LPBYTE	colorData;		// ԭʼλͼ��ɫ�Ļ�����
	DWORD	colorSize;		// ԭʼλͼ��ɫ���ݵĴ�С
	DWORD	lineSize;		// ÿ����ɫ���ݵĴ�С

	// �ļ�ͷ
	LPBITMAPFILEHEADER lpFileHeader = (LPBITMAPFILEHEADER)buffer;
	// ��Ϣͷ
	LPBITMAPINFOHEADER lpInfoHeader = (LPBITMAPINFOHEADER)((LPBYTE)buffer + sizeof(BITMAPFILEHEADER));

	/**
	* ��ȡԭʼλͼ��Ϣ
	*/
	width		= lpInfoHeader->biWidth;
	height		= lpInfoHeader->biHeight;
	colorData	= (LPBYTE)buffer + lpFileHeader->bfOffBits;
	colorSize	= lpFileHeader->bfSize - lpFileHeader->bfOffBits;
	lineSize	= colorSize / height;

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
			Color* color = (Color*)(colorData + index);
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

/**
* @brief ת��λͼ��Ϣ
* @return ת�����λͼ��Ϣ��ʹ�����ǵ��ͷ�
*/
LPVOID transform(LPVOID lpSource, LPRECT lpRect, LPDWORD lpTransformSize)
{
	DWORD	width;				// ԭʼλͼ�Ŀ��
	DWORD	height;				// ԭʼλͼ�ĸ߶�
	LPBYTE	colorData;			// ԭʼλͼ��ɫ���ݻ�����
	DWORD	colorSize;			// ԭʼλͼ��ɫ���ݵĴ�С
	DWORD	lineSize;			// ԭʼλͼÿ����ɫ���ݵĴ�С

								// �ļ�ͷ
	LPBITMAPFILEHEADER lpFileHeader = (LPBITMAPFILEHEADER)lpSource;
	// ��Ϣͷ
	LPBITMAPINFOHEADER lpInfoHeader = (LPBITMAPINFOHEADER)((LPBYTE)lpSource + sizeof(BITMAPFILEHEADER));

	/**
	* ��ȡԭʼλͼ��Ϣ
	*/
	width = lpInfoHeader->biWidth;
	height = lpInfoHeader->biHeight;
	colorSize = lpFileHeader->bfSize - lpFileHeader->bfOffBits;
	colorData = (LPBYTE)lpSource + lpFileHeader->bfOffBits;
	lineSize = colorSize / height;

	DWORD newWidth = lpRect->right - lpRect->left;			// ��λͼ�Ŀ��
	DWORD newHeight = lpRect->top - lpRect->bottom;			// ��λͼ�ĸ߶�

	DWORD newLineSize = newWidth * 3;					// ��λͼÿ����ɫ���ݵĴ�С
	if (newLineSize % 4 != 0) {
		int _tmp = newLineSize % 4;
		newLineSize += 4 - _tmp;
	}
	DWORD newColorSize = newHeight*newLineSize;						// ��λͼ��ɫ���ݵĴ�С
	DWORD newFileSize = newColorSize + lpFileHeader->bfOffBits;		// ��λͼ�ļ���С
	BYTE* newColorData = new BYTE[newColorSize];					// ��λͼ��ɫ���ݻ�����

	/**
	* �޸��ļ�ͷ����Ϣͷ
	*/
	lpFileHeader->bfSize = newFileSize;
	lpFileHeader->bfOffBits = 54;
	lpInfoHeader->biWidth = newWidth;
	lpInfoHeader->biHeight = newHeight;
	lpInfoHeader->biSizeImage = newColorSize;

	// ��ʼ�����������
	for (int line = 0; line < newHeight; line++)
	{
		// �������
		LPBYTE linePoint = newColorData + (line*newLineSize);
		LPBYTE sourcePoint = colorData + (line + lpRect->bottom)*lineSize + lpRect->left * 3;
		CopyMemory(linePoint, sourcePoint, newLineSize);
	}

	/**
	* �����ļ����沢�ͷ���Ӧ���ڴ�
	*/
	LPBYTE newBmpBuffer = new BYTE[newFileSize];
	CopyMemory(newBmpBuffer, lpFileHeader, 54);
	CopyMemory(newBmpBuffer + 54, newColorData, newColorSize);
	delete[] newColorData;
	*lpTransformSize = newFileSize;

	// ����λͼ����
	return newBmpBuffer;
}

void transformBmp(LPCSTR inputFileName,LPCSTR outputFileName)
{
	DWORD size;
	RECT rect;
	LPBYTE buffer = (LPBYTE)readFormFile(inputFileName, &size);
	getRect(buffer, &rect);
	LPBYTE newBmpBuffer = (LPBYTE)transform(buffer, &rect, &size);
	writeToFile(outputFileName, newBmpBuffer, size);
	delete[] buffer;
	delete[] newBmpBuffer;
	Sleep(100);
}

/**
* @brief ����λͼ
*/
void generateBmpList()
{
	for (int i = 1; i <= 126; i++)
	{
		stringstream in;
		in << "C:/Users/Administrator/Desktop/dinosaur-bmp/dinosaur (";
		in << i;
		in << ").bmp";
		string inputFileName = in.str();

		stringstream out;
		out << "D:/FFOutput/dinosaur/dinosaur";
		out << i;
		out << ".bmp";
		string outputFileName = out.str();
		transformBmp(inputFileName.c_str(), outputFileName.c_str());
		printf("����λͼ�ɹ���%s\n",outputFileName.c_str());
	}
}

int main()
{
	generateBmpList();
	system("pause");
	return 0;
}
