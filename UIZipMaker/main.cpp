#include <iostream>
#include <shlwapi.h>

#include "stdstring.h"
#include "file/FileEnumerator.h"
#include "QBuffer.h"
#include "crypt/QDataEnDe.h"
#include "crypt/CXUnzip.h"

using namespace std;

bool PackUIZip(LPCWSTR szDir)
{
    FileList lstFiles;
    QFileFinder(szDir, L"*.*", TRUE).GetFileList(lstFiles);
    if (lstFiles.size() < 0)
    {
        wcout<<"No files found!\n";
        return false;
    }

    CStdString sDir(szDir);
    // get the password
    QBuffer qBuf;
    if (!qBuf.FileRead(sDir + L"CODE"))
    {
        wcout<<L"No CODE file!\n";
        return false;
    }
    QDataEnDe cEnDe;
    if (!cEnDe.SetCodeData(qBuf))
    {
        wcout<<"Failed Init DataEnDe"<<endl;
        return false;
    }

    CXZip zip;
    if (!zip.Create( sDir + L"Main.zip"))
    {
        wcout<<"Create Zip Failed!"<<endl;
        return false;
    }
    // Code File;
    zip.AddFile(L"Code",qBuf.GetBuffer(0),qBuf.GetBufferLen());

    // pack
    CStdString sFile;
    int nDirLen = sDir.GetLength();
    for (FileListItr i = lstFiles.begin(); i != lstFiles.end(); ++i)
    {
        //fX.Read(buf,3); // 丢弃文件的编码信息,要求所有的文本文件必须为UTF-8格式
        if (qBuf.FileRead((*i).c_str()))
        {
            sFile = *i;
            sFile = sFile.Mid(nDirLen);
            cEnDe.EncryptData(qBuf);
            zip.AddFile(sFile, qBuf.GetBuffer(0), qBuf.GetBufferLen());
            wcout<<"pack: "<<sFile<<"  ----> done!"<<endl;
        }
        else
        {
            wcout<<"pack: "<<*i<<"  ----> failed!"<<endl;
        }
    }
    // 把CODE重写
    qBuf.FileRead(sDir + L"CODE");
    zip.AddFile(sDir + L"CODE", qBuf.GetBuffer(0), qBuf.GetBufferLen());
    return true;
}

bool UnpackUIzip(LPCWSTR pszZip)
{
    wcout<<"The function doesn't implet!"<<endl;
    return false;
}

int wmain(int nArgc, wchar_t** argv)
{
    if (nArgc < 3)
    {
        wcout<<"usage: \n"
            <<" 1>pack:ui c:\\dir\\ -p (need last backslash)\n"
            <<" 2>unpack:ui c:\\dir\\f.zip -u (will overwrite files in c:\\dir)\n"
            <<endl;
        return 0;
    }

    if (StrCmpIW(argv[2], L"-p") == 0)
    {
        PackUIZip(argv[1]);
    }
    else if (StrCmpIW(argv[2], L"-u") == 0)
    {
        UnpackUIzip(argv[1]);
    }

    return 0;
}


