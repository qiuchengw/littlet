//------------------------------------------------------------------------------
//      Author:	Martin Johannes Isaak Brandl 
//              mjibrandl@googlemail.com
//
// Description: Determines the file version of a specific file.
//
//        Date: 05.05.2010
//
//     License: This file is Copyright ?2010 Martin Johannes Isaak Brandl.
//				All Rights Reserved.
//              This software is released under the Code Project Open License 
//              (CPOL), which may be found here:  
//				http://www.codeproject.com/info/eula.aspx
//------------------------------------------------------------------------------
#include "FileVersion.hpp"

#include <Windows.h>
#include <vector>

// Look for version.lib
#pragma comment( lib, "version.lib" )

// Default format string when GetFileVersion() is called with 1 argument
// const TCHAR FileVersion::kVersionFormat[]	= _T("%i.%i.%i.%i");

FileVersion::FileVersion( const CStdString & kstrFilePath )
: mbVersionIsValid ( false )
{
	mbVersionIsValid = DetermineFileVersion( kstrFilePath );
}


FileVersion::FileVersion()
: mbVersionIsValid ( false )
{
	wchar_t	strTempBuffer[ MAX_PATH ];
	// Retrieve the fully-qualified path from the current process
	if ( NULL == ::GetModuleFileName( NULL	// Current module
		, strTempBuffer
		, MAX_PATH ) )
		return;	// Can't retrieve path from current process

	mbVersionIsValid = DetermineFileVersion( CStdString(strTempBuffer) );
}

FileVersion::~FileVersion(void)
{
}

bool FileVersion::DetermineFileVersion( const CStdString& kstrFilePath )
{
	// Precondition
	if ( kstrFilePath.IsEmpty() )
		return false; // FilePath is empty, no file to determine version.

	DWORD	dwHandle;

    LPCWSTR ch_file_path = (LPCWSTR)kstrFilePath;
	// Determines whether the operating system can retrieve version information
	// for a specified file.
	DWORD	dwFileVersionInfoSize = 
        GetFileVersionInfoSize(ch_file_path, &dwHandle);

	if ( NULL == dwFileVersionInfoSize )
		return false;	// Can't retrieve version information size.

	// Allocate space to retrieve version information using vector to prevent
	// memory leaks
	std::vector<BYTE>	pData( dwFileVersionInfoSize );

	// Retrieves version information for the specified file.
    if (false == GetFileVersionInfo(ch_file_path
		, dwHandle
		, dwFileVersionInfoSize
		, static_cast<LPVOID>( &pData[0] ) ) )
		return false; // Can't retrieve version information.

	// The memory of ptFileInfo is freed when pData is freed.
	VS_FIXEDFILEINFO *ptFileInfo;
	UINT	uintSize;

	// Retrieves version information from the version-information resource
	if ( false == VerQueryValue( static_cast<LPVOID>( &pData[0] )
		, L"\\"
		, reinterpret_cast<LPVOID*> ( &ptFileInfo )
		, &uintSize ) )
		return false; // Can't retrieve version information

	// Resolve major, minor, release and build number.
	musMajorVersion		= static_cast<unsigned short>(  
		( ptFileInfo->dwFileVersionMS >> 16 ) &0xffff );

	musMinorVersion		= static_cast<unsigned short>( 
		ptFileInfo->dwFileVersionMS &0xffff );

	musReleaseNumber	= static_cast<unsigned short>( 
		( ptFileInfo->dwFileVersionLS >> 16 ) &0xffff);

	musBuildNumber		= static_cast<unsigned short>( 
		ptFileInfo->dwFileVersionLS & 0xffff );

	return true;
}


CStdString FileVersion::GetFileVersion() const
{
    if (false == mbVersionIsValid)
        return ""; // This version is not valid.

    CStdString s;
    s.Format(L"%u.%u.%u.%u", musMajorVersion, musMinorVersion, musReleaseNumber, musBuildNumber);
       // .toStdString();
    return s;
}


bool FileVersion::IsEqual( const FileVersion & krhs ) const
{
	if ( musMajorVersion != krhs.musMajorVersion )
		return false;

	if ( musMinorVersion != krhs.musMinorVersion )
		return false;

	if ( musReleaseNumber != krhs.musReleaseNumber )
		return false;

	return musBuildNumber == krhs.musBuildNumber;
}


// Comparator: Less then
bool FileVersion::operator<( const FileVersion & krhs ) const
{
	if ( musMajorVersion < krhs.musMajorVersion )
		return true;

	if ( musMajorVersion > krhs.musMajorVersion )
		return false;

	if ( musMinorVersion < krhs.musMinorVersion )
		return true;

	if ( musMinorVersion > krhs.musMinorVersion )
		return false;

	if ( musReleaseNumber < krhs.musReleaseNumber )
		return true;

	if ( musReleaseNumber > krhs.musReleaseNumber )
		return false;

	return musBuildNumber < krhs.musBuildNumber;
}


// Comparator: Greater then
bool FileVersion::operator>( const FileVersion & krhs ) const
{
	if ( true == this->IsEqual( krhs) )
		return false; // Equal file version

	// File version is greater if its not equal and not less then passed 
	// version.
	return ( false == ( this->operator<( krhs ) ) );
}


// Operator: Equal
bool FileVersion::operator==( const FileVersion & krhs ) const
{
	return IsEqual( krhs );
}

CStdString FileVersion::GetMajorMinorVersion() const
{
    CStdString s;
    s.Format(L"%u.%u", musMajorVersion, musMinorVersion);
    return s;
}



