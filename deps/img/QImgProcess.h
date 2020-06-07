#pragma once

#include <GdiPlus.h>

class QBuffer;
class QImgProcess
{
public:
	QImgProcess(void);
	~QImgProcess(void);

	// 新的宽高，有可能变形
	// bFailIfSmall = TRUE,如果当前图片的宽高都小于要创建的缩略图大小，
	//			那么就不创建缩略图
	// 返回新创建的缩略图指针，这个指针要手动删除，使用delete
	Gdiplus::Image* CreateThumb( int cx ,int cy,BOOL bFailIfSmall);
	// 保持宽高比
	Gdiplus::Image* CreateThumb( int cx ,BOOL bFailIfSmall);
	// open image file
	BOOL OpenFile(LPCTSTR sFile);
	void Close();

	BOOL Save(LPCTSTR pszPath,int quality =100);
// 	BOOL TransformFormat(LPCTSTR pszFormat,int quality=100);
// 	BOOL GetImageData( __out QBuffer& buf );
// 	DWORD GetImagePhysicSize();

public:
	static BOOL SaveIamge(Gdiplus::Image *pImg,LPCTSTR pszPath,int quality =100);
// 	static BOOL GetImageData( Gdiplus::Image*pImg,__out QBuffer& buf );
// 	static BOOL TransformFormat(Gdiplus::Image *pImg,LPCTSTR pszFormat,int quality=100);
// 	static DWORD GetImagePhysicSize(Gdiplus::Image* pImg);
private:
	Gdiplus::Image*		m_pImg;
};
