#pragma once
/**
* @brief λͼ������
*/
class BmpUtil
{
public:
	BmpUtil();
	~BmpUtil();

	/*
	*	@brief λͼ��Ϣ
	*
	*/
	struct BmpInfo
	{
		/**
		 * @brief λͼ��С
		 */
		long bmpSize;

		/**
		* @brief λͼ����
		*/
		void* bmpData;
	};

	/*
	* @brief ת��λͼ
	* @param sourceData ԭbmp������Ϣ
	* @return λͼ��Ϣ
	*/
	BmpInfo* transformBmp(void* sourceData);
};

