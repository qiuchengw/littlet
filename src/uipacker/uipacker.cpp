#define _ATL_EX_CONVERSION_MACROS_ONLY
#define SS_NO_CONVERSION

#include "file/FileEnumerator.h"
#include "crypt/CXUnzip.h"
#include "BaseType.h"
#include "crypt/rijndael.h"
#include "crypt/QDataEnDe.h"
#include <iostream>

void msg(const std::wstring& msg) {
	std::wcout<< msg << std::endl;
}

bool PackUIZip(const CStdString& sDir, const CStdString& sDest)
{
	FileList lstFiles;
	QFileFinder(sDir, L"*.*", TRUE).GetFileList(lstFiles);
	if (lstFiles.size() < 0)
	{
		msg(L"No files found!\n");
		return false;
	}

	// get the password
	QBuffer qBuf;
	if (!qBuf.FileRead(sDir + L"CODE"))
	{
		msg(L"No CODE file!\n");
		return false;
	}
	QDataEnDe cEnDe;
	if (!cEnDe.SetCodeData(qBuf))
	{
		msg(L"Failed Init DataEnDe");
		return false;
	}

	CXZip zip;
	if (!zip.Create(sDest))
	{
		msg(L"Create Zip Failed!");
		return false;
	}
	// Code File;
	zip.AddFile(L"Code", qBuf.GetBuffer(0), qBuf.GetBufferLen());

	// pack
	CStdString sFile;
	int nDirLen = sDir.GetLength();
	for (FileListItr i = lstFiles.begin(); i != lstFiles.end(); ++i)
	{
		if (qBuf.FileRead((*i).c_str()))
		{
			sFile = *i;
			sFile = sFile.Mid(nDirLen);
			cEnDe.EncryptData(qBuf);
			zip.AddFile(sFile, qBuf.GetBuffer(0), qBuf.GetBufferLen());
		}
	}
	return true;
}

int main(int argc, const char** argv){
	std::cout << "argc:" << argc << std::endl;
	if (argc != 3) {
		std::cout << "usage: uipacker $ui_dir $dest_file";
		return -1;
	}
	PackUIZip(argv[1], argv[2]);
	return 0;
}