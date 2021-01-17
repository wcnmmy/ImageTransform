#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include <sstream>
using namespace std;

//
// 这是背景颜色
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
* @brief 读取文件内容
*		请自行释放内存
* @param fileName 文件名
* @param lpReaddedSize 实际读取到的大小
* @return 读取到的内容的缓存区，（请自行释放内存）
*/
LPVOID readFormFile(LPCSTR fileName, LPDWORD lpReadedSize)
{
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;		// 文件信息
	DWORD fileSize;							// 文件大小

	HANDLE hFile = CreateFileA(fileName
		, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//printf("打开文件失败！\n");
		return NULL;
	}
	// 获取文件大小
	GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileInfo);
	fileSize = fileInfo.nFileSizeLow;
	
	LPBYTE readBuffer = new BYTE[fileSize];
	ReadFile(hFile, readBuffer, fileSize, lpReadedSize, NULL);
	CloseHandle(hFile);
	return readBuffer;
}

/**
* @brief 将指定缓存区的内存写到文件中
* @param outputFileName 输出的文件名
* @param buffer 缓存区
* @param size 要写入的大小
* @return 实际写入的大小
*/
DWORD writeToFile(LPCSTR outputFileName,LPVOID buffer,DWORD size)
{
	DWORD dwWriteSize;
	HANDLE hFile = CreateFileA(outputFileName
		, GENERIC_WRITE | GENERIC_READ, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	BOOL bRet = WriteFile(hFile, buffer, size, &dwWriteSize, NULL);
	if (bRet)
	{
		//printf("文件写入成功！\n");
	}
	else
	{
		printf("文件写入失败!文件大小:%d\n",size);
	}
	CloseHandle(hFile);
	return size;
}

/**
* @brief 获取图片要裁剪的区域
* @param buffer 图片数据的缓存区
* @param lpRect 裁剪区域的指针
*/
void getRect(LPVOID buffer,LPRECT lpRect)
{
	DWORD	width;			// 原始位图宽度
	DWORD	height;			// 原始位图高度
	LPBYTE	colorData;		// 原始位图颜色的缓存区
	DWORD	colorSize;		// 原始位图颜色数据的大小
	DWORD	lineSize;		// 每行颜色数据的大小

	// 文件头
	LPBITMAPFILEHEADER lpFileHeader = (LPBITMAPFILEHEADER)buffer;
	// 信息头
	LPBITMAPINFOHEADER lpInfoHeader = (LPBITMAPINFOHEADER)((LPBYTE)buffer + sizeof(BITMAPFILEHEADER));

	/**
	* 获取原始位图信息
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

	// 开始遍历
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int index = y*lineSize + x * 3;
			Color* color = (Color*)(colorData + index);
			COLORREF colorref = color->hex();
			if (colorref != bcColor)
			{
				// 这个颜色不是背景颜色
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
* @brief 转换位图信息
* @return 转换后的位图信息，使用完后记得释放
*/
LPVOID transform(LPVOID lpSource, LPRECT lpRect, LPDWORD lpTransformSize)
{
	DWORD	width;				// 原始位图的宽度
	DWORD	height;				// 原始位图的高度
	LPBYTE	colorData;			// 原始位图颜色数据缓存区
	DWORD	colorSize;			// 原始位图颜色数据的大小
	DWORD	lineSize;			// 原始位图每行颜色数据的大小

								// 文件头
	LPBITMAPFILEHEADER lpFileHeader = (LPBITMAPFILEHEADER)lpSource;
	// 信息头
	LPBITMAPINFOHEADER lpInfoHeader = (LPBITMAPINFOHEADER)((LPBYTE)lpSource + sizeof(BITMAPFILEHEADER));

	/**
	* 获取原始位图信息
	*/
	width = lpInfoHeader->biWidth;
	height = lpInfoHeader->biHeight;
	colorSize = lpFileHeader->bfSize - lpFileHeader->bfOffBits;
	colorData = (LPBYTE)lpSource + lpFileHeader->bfOffBits;
	lineSize = colorSize / height;

	DWORD newWidth = lpRect->right - lpRect->left;			// 新位图的宽度
	DWORD newHeight = lpRect->top - lpRect->bottom;			// 新位图的高度

	DWORD newLineSize = newWidth * 3;					// 新位图每行颜色数据的大小
	if (newLineSize % 4 != 0) {
		int _tmp = newLineSize % 4;
		newLineSize += 4 - _tmp;
	}
	DWORD newColorSize = newHeight*newLineSize;						// 新位图颜色数据的大小
	DWORD newFileSize = newColorSize + lpFileHeader->bfOffBits;		// 新位图文件大小
	BYTE* newColorData = new BYTE[newColorSize];					// 新位图颜色数据缓存区

	/**
	* 修改文件头和信息头
	*/
	lpFileHeader->bfSize = newFileSize;
	lpFileHeader->bfOffBits = 54;
	lpInfoHeader->biWidth = newWidth;
	lpInfoHeader->biHeight = newHeight;
	lpInfoHeader->biSizeImage = newColorSize;

	// 开始遍历并且填充
	for (int line = 0; line < newHeight; line++)
	{
		// 逐行填充
		LPBYTE linePoint = newColorData + (line*newLineSize);
		LPBYTE sourcePoint = colorData + (line + lpRect->bottom)*lineSize + lpRect->left * 3;
		CopyMemory(linePoint, sourcePoint, newLineSize);
	}

	/**
	* 构建文件缓存并释放相应的内存
	*/
	LPBYTE newBmpBuffer = new BYTE[newFileSize];
	CopyMemory(newBmpBuffer, lpFileHeader, 54);
	CopyMemory(newBmpBuffer + 54, newColorData, newColorSize);
	delete[] newColorData;
	*lpTransformSize = newFileSize;

	// 返回位图数据
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
* @brief 生成位图
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
		printf("生成位图成功：%s\n",outputFileName.c_str());
	}
}

int main()
{
	generateBmpList();
	system("pause");
	return 0;
}
