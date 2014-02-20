#include "MainWnd.h"
#include "../common/LittleTUIcmn.h"
#include "ui/QUIMgr.h"
#include "ui/QUIGlobal.h"
#include "ui/quimgr.h"
#include "crypt/QDataEnDe.h"
#include "file/FileOper.h"
#include "crypt/CXUnzip.h"
#include "file/FileEnumerator.h"

QUI_BEGIN_EVENT_MAP(MainWnd,QFrame)
	BN_CLICKED_ID(L"btn_encrypt",&MainWnd::OnBtnEncrypt)
	BN_CLICKED_ID(L"btn_decrypt",&MainWnd::OnBtnDecrypt)
QUI_END_EVENT_MAP()

MainWnd::MainWnd(void)
	:QFrame(L"qabs:Main.htm")
{
}

MainWnd::~MainWnd(void)
{
}

BOOL MainWnd::Startup()
{
	return QFrame::Create(NULL);
}

BOOL MainWnd::OnClose()
{
    SetMsgHandled(FALSE);
	if (XMsgBox::YesNoMsgBox(L"确定要退出程序么？") == IDYES)
	{
		::DestroyWindow(GetSafeHwnd());
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

BOOL MainWnd::on_key( HELEMENT he, HELEMENT target, 
	UINT event_type, UINT code, UINT keyboardStates )
{
	if (KEY_UP == event_type)
	{
		if (VK_RETURN == code)
		{
			ECtrl ctl(he);
			if (aux::wcseqi(ctl.get_attribute("id"),L"search-bar"))
			{ 
				return TRUE;
 			}
		}
	}
	return FALSE;
}

BOOL MainWnd::ZipData(__in const QString&sDataPath,__out QBuffer &bufData)
{
	// 扫描文件
	QBuffer bufFile;
	QFileFinder ff(sDataPath,L"*.*");
	FileList fileList;
	ff.GetFileList(fileList);

	// 创建压缩文件
	CXZip zip;
	QString sTempZip = sDataPath + L"/__temp_file.zip";
	if (!zip.Create(sTempZip))
		return FALSE;
	// 写入压缩文件
	for (FileListItr itr = fileList.begin();
		itr != fileList.end(); ++itr)
	{
		if (!bufFile.FileRead((*itr).c_str()))
		{
			return FALSE;
		}
		zip.AddFile(CPath((*itr).c_str()).GetFileName(),
			bufFile.GetBuffer(0),bufFile.GetBufferLen());
		bufFile.ClearBuffer();
	}
	zip.Close();
	
	BOOL bRet = bufData.FileRead(sTempZip);
	DeleteFile(sTempZip);
	return bRet;
}

void MainWnd::OnBtnEncrypt( HELEMENT )
{
	QString sPicPath = EFilePath(GetCtrl("#path_picture")).GetFilePath();
	QString sDataPath = EFolderPath(GetCtrl("#path_data")).GetFolderPath();
	
	ECtrl eInfo = GetCtrl("#infobox");
	if (sPicPath.IsEmpty() || sDataPath.IsEmpty())
	{
		eInfo.SetText(L"请填写完整的信息");
		return;
	}

	QBuffer bufPic;
	if (!bufPic.FileRead(sPicPath))
	{
		eInfo.SetText(L"图像文件读取错误");
		return ;
	}
	DWORD dwPicSize = bufPic.GetBufferLen();
	if (dwPicSize < 2048)
	{
		eInfo.SetText(L"图像文件太小了");
		return;
	}

	ENumber ePicOffset = GetCtrl("#input_pic_offset");
	ePicOffset.SetLimit(0,bufPic.GetBufferLen() / 2);
	ENumber eKeyLen = GetCtrl("#input_pic_keylen");

	// 读取文件夹下的所有文件
	// 并创建压缩文件	
	QBuffer bufData;
	if (!ZipData(sDataPath,bufData))
	{
		eInfo.SetText(L"创建数据压缩文件错误");
		return;
	}
	DWORD dwOriginLen = bufData.GetBufferLen();
	DWORD dwOffset = ePicOffset.GetNum();
	DWORD dwKeyLen = eKeyLen.GetNum();
	QDataEnDe ende;
	if (!ende.SetCodeData(bufPic.GetBuffer(dwOffset),dwKeyLen))
	{
		eInfo.SetText(L"设置密码数据失败");
		return;
	}
	if (!ende.EncryptData(bufData))
	{
		eInfo.SetText(L"加密数据失败!");
		return;
	}
	DWORD dwEncrypDataLen = bufData.GetBufferLen();

	if (!bufPic.Write(bufData.GetBuffer(),dwEncrypDataLen))
	{
		eInfo.SetText(L"写入数据失败");
		return;
	}
	// 最后写入20个字节，顺序如下
	// 原始数据长度
	bufPic.Write((BYTE*)&dwOriginLen,sizeof(DWORD));
	// 加密后的数据长度
	bufPic.Write((BYTE*)&dwEncrypDataLen,sizeof(DWORD));
	// 图像大小
	bufPic.Write((BYTE*)&dwPicSize,sizeof(DWORD));
	// 密钥长度
	bufPic.Write((BYTE*)&dwKeyLen,sizeof(DWORD));
	// 密钥在图像数据中的偏移
	bufPic.Write((BYTE*)&dwOffset,sizeof(DWORD));

	if (!bufPic.FileWrite(sDataPath + L"/" + CPath(sPicPath).GetFileName()))
	{
		eInfo.SetText(L"保存文件失败!");
		return;
	}
	eInfo.SetText(L"全部搞定啦！尽可使用");
}

void MainWnd::OnBtnDecrypt( HELEMENT )
{
	QString sPicPath = EFilePath(GetCtrl("#path_picture")).GetFilePath();
	QString sSavePath = EFolderPath(GetCtrl("#path_data")).GetFolderPath();

	ECtrl eInfo = GetCtrl("#infobox");
	if (sPicPath.IsEmpty() || sSavePath.IsEmpty())
	{
		eInfo.SetText(L"请填写完整的信息");
		return;
	}

	QBuffer bufPic;
	if (!bufPic.FileRead(sPicPath))
	{
		eInfo.SetText(L"图像文件读取错误");
		return ;
	}
	DWORD dwPicSize = bufPic.GetBufferLen();
	if (dwPicSize < 2048)
	{
		eInfo.SetText(L"错误的加密图像文件");
		return;
	}


	// 最后写入20个字节，顺序如下

	// 密钥在图像数据中的偏移
	DWORD dwOffset;
	bufPic.ReadLast((BYTE*)&dwOffset,sizeof(DWORD));
	// 密钥长度
	DWORD dwKeyLen;
	bufPic.ReadLast((BYTE*)&dwKeyLen,sizeof(DWORD));
	// 图像大小
	bufPic.ReadLast((BYTE*)&dwPicSize,sizeof(DWORD));
	// 加密后的数据长度
	DWORD dwEncrypDataLen;
	bufPic.ReadLast((BYTE*)&dwEncrypDataLen,sizeof(DWORD));
	// 原始数据长度
	DWORD dwOriginLen;
	bufPic.ReadLast((BYTE*)&dwOriginLen,sizeof(DWORD));

	QBuffer bufData;
	if (!bufData.AllocBuffer(dwEncrypDataLen))
	{
		eInfo.SetText(L"申请内存错误");
		return;
	}
	if (!bufData.Write(bufPic.GetBuffer(dwPicSize),dwEncrypDataLen))
	{
		eInfo.SetText(L"读取数据错误");
	}
	
	QDataEnDe ende;
	if (!ende.SetCodeData(bufPic.GetBuffer(dwOffset),dwKeyLen))
	{
		eInfo.SetText(L"设置密码数据失败");
		return;
	}
	if (!ende.DecryptData(bufData))
	{
		eInfo.SetText(L"解密数据失败!");
		return;
	}
	QString sFileName = L"/__Data__.dat";
	if (!bufData.FileWrite(sSavePath + sFileName))
	{
		eInfo.SetText(L"保存数据失败");
		return;
	}

	eInfo.SetText(L"全部搞定啦！已经保存为：" + sFileName);
}

