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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef FILEVERSION_H
#define FILEVERSION_H

#include "stdstring.h"

// Macro to disallow the copy constructor and operator= function
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&);             \
	void operator=(const TypeName&) 
#endif // DISALLOW_COPY_AND_ASSIGN


class FileVersion
{
public:
	explicit FileVersion( const CStdString& kstrFilePath );
	FileVersion( void );	// Use current process path
	~FileVersion( void );


// Functions
public:
	// Return whether the file version could been determined.
	bool IsValid()						const { return mbVersionIsValid; };

	unsigned short GetMajorVersion()	const { return musMajorVersion;  };
	unsigned short GetMinorVersion()	const { return musMinorVersion;  };
	unsigned short GetReleaseNumber()	const { return musReleaseNumber; };
	unsigned short GetBuildNumber()		const { return musBuildNumber;   };

	// Get the file version string. The order is Major, Minor, Release and Build
	// Number. The string can be formated, for instance: L"Version: %i.%i.%i.%i"
	// will return a string like "Version: 12.1.0.530". 
	
	// Get the file version string. The order is Major, Minor, Release and Build
	// Number with a dot between. For instance: "12.1.0.530".
    CStdString GetFileVersion() const;

    CStdString GetMajorMinorVersion() const;

	// Return true if the file version is equal to the passed file version.
	bool IsEqual ( const FileVersion & krhs ) const;

	// Return true if the file version is less then the passed file version.
	bool operator<( const FileVersion & krhs ) const;

	// Return true if the file version is greater then the passed file version.
	bool operator>( const FileVersion & krhs ) const;

	// Return true if the file version is equal to the passed file version.
	bool operator==( const FileVersion & krhs ) const;


private:
	// This helper method determines the file version. It is called from the 
	// constructor.
    bool DetermineFileVersion(const CStdString& kstrFilePath);

// Attributes
private:
	unsigned short	musMajorVersion;
	unsigned short	musMinorVersion;
	unsigned short	musReleaseNumber;
	unsigned short	musBuildNumber;

	bool			mbVersionIsValid;

	static const CStdString kVersionFormat[];

	
private:
	DISALLOW_COPY_AND_ASSIGN( FileVersion );
};


#endif // FILEVERSION_H