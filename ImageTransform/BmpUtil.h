#pragma once
/**
* @brief 位图工具类
*/
class BmpUtil
{
public:
	BmpUtil();
	~BmpUtil();

	/*
	*	@brief 位图信息
	*
	*/
	struct BmpInfo
	{
		/**
		 * @brief 位图大小
		 */
		long bmpSize;

		/**
		* @brief 位图数据
		*/
		void* bmpData;
	};

	/*
	* @brief 转换位图
	* @param sourceData 原bmp数据信息
	* @return 位图信息
	*/
	BmpInfo* transformBmp(void* sourceData);
};

