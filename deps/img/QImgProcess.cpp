#include "stdafx.h"
#include "QImgProcess.h"
#include "QBuffer.h"
#include "file/QFile.h"
#include "deps/file/FileOper.h"

using namespace Gdiplus;

namespace quibase
{
    int   GetEncoderClsid(__in const   WCHAR*   format, __inout CLSID*   pClsid)
    {
        UINT     num = 0;                     //   number   of   image   encoders 
        UINT     size = 0;                   //   size   of   the   image   encoder   array   in   bytes 

        ImageCodecInfo*   pImageCodecInfo = NULL;

        GetImageEncodersSize(&num, &size);
        if (size == 0)
            return   -1;     //   Failure 

        pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == NULL)
            return   -1;     //   Failure 

        GetImageEncoders(num, size, pImageCodecInfo);

        for (UINT j = 0; j < num; ++j)
        {
            if (wcsicmp(pImageCodecInfo[j].MimeType, format) == 0)
            {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return   j;     //   Success 
            }
        }

        free(pImageCodecInfo);
        return   -1;     //   Failure 
    }

}

QImgProcess::QImgProcess(void)
{
	m_pImg = NULL;
}

QImgProcess::~QImgProcess(void)
{
	Close();
}

Image* QImgProcess::CreateThumb( int cx ,int cy,BOOL bFailIfSmall)
{
	if ((0 >= cx) || ( 0 >= cy ) || (nullptr == m_pImg))
	{
		ATLASSERT(FALSE);
		return NULL;
	}
	// 创建缩略图
	int nWidth = m_pImg->GetWidth();
	int nHeight = m_pImg->GetHeight();
	if (bFailIfSmall && (nWidth <= cx) && (nHeight <= cy))
	{
		return NULL;
	}

	// 使用32ARGB使得背景透明
	Bitmap* pBitmap = new Bitmap(cx, cy, PixelFormat32bppARGB);  // PixelFormat24bppRGB
	Graphics graph(pBitmap);
	if (graph.DrawImage(m_pImg, Rect(0,0,cx,cy))!= Ok)
	{
		delete pBitmap;
		return NULL;
	}
	return pBitmap;
}

Image* QImgProcess::CreateThumb( int cx ,BOOL bFailIfSmall )
{
	ATLASSERT(m_pImg != NULL);
	// 创建缩略图
	return CreateThumb(cx,(m_pImg->GetHeight()) * cx / m_pImg->GetWidth(),bFailIfSmall);
}

BOOL QImgProcess::OpenFile( LPCTSTR sFile )
{
	Close();
	m_pImg = Bitmap::FromFile(sFile);
	return NULL != m_pImg;
}

void QImgProcess::Close()
{
	if (NULL != m_pImg)
	{
		delete m_pImg;
		m_pImg = NULL;
	}
}

BOOL QImgProcess::SaveIamge( Image *pImg,LPCTSTR pszPath,int quality )
{
    if (nullptr == pImg)
    {
    	ATLASSERT(FALSE);
        return FALSE;
    }

	// 保存到文件
	EncoderParameters encoderParameters;  
	//构造编码参数列表  
	//数组中只包含一个EncoderParameter对象  
	encoderParameters.Count = 1;  
	encoderParameters.Parameter[0].Guid = EncoderQuality;  
	//参数类型为LONG  
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;  
	//只设置一个参数 
	encoderParameters.Parameter[0].NumberOfValues = 1;  

	//压缩JPEG图片质量  
	encoderParameters.Parameter[0].Value = &quality;  
	// 保存到文件
	// 解码器
	CLSID clsid;
	CStdString sExtName = quibase::CPath(pszPath).GetExtName();
	if (L"jpg" == sExtName)
		sExtName = L"jpeg";
	if (-1 == quibase::GetEncoderClsid(L"image/" + sExtName, &clsid))
	{
		return FALSE;
	}
	return (Ok == pImg->Save(pszPath,&clsid,&encoderParameters));
}

BOOL QImgProcess::Save( LPCTSTR pszPath,int quality /*=100*/ )
{
	return QImgProcess::SaveIamge(m_pImg,pszPath,quality);
}


// BOOL QImgProcess::TransformFormat(LPCTSTR pszFormat,int quality)
// {
// 	return TransformFormat(m_pImg,pszFormat,quality);
// }
// 
// BOOL QImgProcess::TransformFormat(Image *pImg,LPCTSTR pszFormat,int quality)
// {
// 	ASSERT(pImg != NULL);
// 	EncoderParameters encoderParameters;  
// 	//构造编码参数列表  
// 	//数组中只包含一个EncoderParameter对象  
// 	encoderParameters.Count = 1;  
// 	encoderParameters.Parameter[0].Guid = EncoderQuality;  
// 	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;  
// 	encoderParameters.Parameter[0].NumberOfValues = 1;  
// 	encoderParameters.Parameter[0].Value = &quality;  
// 	CLSID clsid;
// 	if (-1 == GetEncoderClsid(QString(L"image/") + pszFormat, &clsid))
// 		return FALSE;
// 
// 	HGLOBAL hRes = NULL;
// 	IStream *pStream = NULL;
// 	BOOL bOK = FALSE;
// 	do 
// 	{
// 		hRes = GlobalAlloc(GMEM_MOVEABLE,GetImagePhysicSize(pImg));
// 		if (NULL == hRes)
// 			break;
// 		// 第二个参数为True,自动释放内存
// 		if ( S_OK != (CreateStreamOnHGlobal(hRes,TRUE,&pStream)))
// 			break;
// 		if (Ok != pImg->Save(pStream,&clsid,&encoderParameters))
// 			break;
// 
// 		// 创建图像,在此操作中pStream增加了引用计数
// 		// image::FromStream 并没有重新申请内存,而是使用了pStream所指向的内存区.
// 		// 在此处就是hRes,根据测试,CreateStreamOnHGlobal的第二个参数为true时,
// 		// delete pImg 后,pStream引用计数减为0, hRes所指向的内存区得到自动释放.
// 		delete pImg;
// 		pImg = Image::FromStream(pStream,TRUE);
// 		pStream->Release();	// pStream 
// 		// 此处不能释放内存,因为pImg是要使用此数据的
// 		// 通过设置CreateStreamOnHGlobal的第二个参数为True可以自动释放此内存
// 		//		GlobalFree(hRes);	
// 		bOK = TRUE;
// 	} while (0);
// 	if (!bOK)
// 	{
// 		if (NULL != hRes)
// 			GlobalFree(hRes);
// 	}
// 	return bOK;
// }
// 
// BOOL Transform()
// {
// 	// load image
// 	Image m_pMemBmp(L"Nice.bmp");
// 
// 	// Create stream with 0 size
// 	IStream* m_pIStream    = NULL;
// 	if(CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&m_pIStream) != S_OK)
// 	{
// 		return FALSE;
// 	}
// 
// 	CLSID pngClsid;
// 	GetEncoderClsid(L"image/png", &pngClsid);
// 
// 	// Setup encoder parameters
// 	ULONG quality = 100;
// 	EncoderParameters encoderParameters;
// 	encoderParameters.Count = 1;
// 	encoderParameters.Parameter[0].Guid = EncoderQuality;
// 	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
// 	encoderParameters.Parameter[0].NumberOfValues = 1;
// 	encoderParameters.Parameter[0].Value = &quality;
// 
// 	//  Save the image to the stream
// 	Status SaveStatus = m_pMemBmp.Save(m_pIStream, &pngClsid, &encoderParameters);
// 	if(SaveStatus != Ok)
// 	{
// 		// this should free global memory used by the stream
// 		m_pIStream->Release();
// 		return FALSE;
// 	}
// 
// 	// get the size of the stream
// 	ULARGE_INTEGER ulnSize;
// 	LARGE_INTEGER lnOffset;
// 	lnOffset.QuadPart = 0;
// 	if(m_pIStream->Seek(lnOffset, STREAM_SEEK_END, &ulnSize) != S_OK)
// 	{
// 		m_pIStream->Release();
// 		return FALSE;
// 	}
// 
// 	// now move the pointer to the beginning of the file
// 	if(m_pIStream->Seek(lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
// 	{
// 		m_pIStream->Release();
// 		return FALSE;
// 	}
// 
// 	// here you can do what ever you want
// 	/*
// 		1. You can use global memory
// 			HGLOBAL hg;
// 			if(GetHGlobalFromStream(pIStream, &hg) = S_OK)
// 			... use hg for something
// 
// 			2. Copy it into some other buffer
// 			char *pBuff = new char[ulnSize.QuadPart];
// 
// 			// Read the stream directly into the buffer
// 			ULONG ulBytesRead;
// 			if(pIStream->Read(pBuff, ulnSize.QuadPart, &ulBytesRead) != S_OK)
// 			{
// 				pIStream->Release();
// 				return;
// 			}
// 	*/
// 
// 	// I am going to save it to the file just so we can 
// 	// load the jpg to a gfx program
// 	CFile fFile;
// 	if(fFile.Open(_T("test.jpg"), CFile::modeCreate | CFile::modeWrite))
// 	{
// 		char *pBuff = new char[ulnSize.QuadPart];
// 
// 		// Read the stream directly into the buffer
// 		ULONG ulBytesRead;
// 		if(m_pIStream->Read(pBuff, ulnSize.QuadPart, &ulBytesRead) != S_OK)
// 		{
// 			m_pIStream->Release();
// 			delete pBuff;
// 			return;
// 		}
// 
// 		fFile.Write(pBuff, ulBytesRead);
// 		fFile.Close();
// 		delete pBuff;
// 	}
// 	else AfxMessageBox(_T("Failed to save data to the disk!"));
// 
// 	// Free memory used by the stream
// 	m_pIStream->Release();
// }

