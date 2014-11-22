#pragma once

/****************************** Module Header ******************************\
    * Module Name:	CppUACSelfElevation.cpp
    * Project:		CppUACSelfElevation
    * Copyright (c) Microsoft Corporation.
    * 
    * User Account Control (UAC) is a new security component in Windows Vista and 
    * newer operating systems. With UAC fully enabled, interactive administrators 
    * normally run with least user privileges. This example demonstrates how to 
    * check the privilege level of the current process, and how to self-elevate 
    * the process by giving explicit consent with the Consent UI. 
    * 
    * This source is subject to the Microsoft Public License.
    * See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
    * All other rights reserved.
    * 
    * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
    * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
    * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
    \***************************************************************************/
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

//////////////////////////////////////////////////////////////////////////
// UAC下为应用程序提权
class CUACSElfElevations
{
public:
    static BOOL SelfElevation();

public:
//#pragma region "Helper Functions for Admin Privileges and Elevation Status"

    //
    //   FUNCTION: IsUserInAdminGroup()
    //
    //   PURPOSE: The function checks whether the primary access token of the 
    //   process belongs to user account that is a member of the local 
    //   Administrators group, even if it currently is not elevated.
    //
    //   RETURN VALUE: Returns TRUE if the primary access token of the process 
    //   belongs to user account that is a member of the local Administrators 
    //   group. Returns FALSE if the token does not.
    //
    //   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
    //   contains the Win32 error code of the failure.
    //
    //   EXAMPLE CALL:
    //     try 
    //     {
    //         if (IsUserInAdminGroup())
    //             wprintf (L"User is a member of the Administrators group\n");
    //         else
    //             wprintf (L"User is not a member of the Administrators group\n");
    //     }
    //     catch (DWORD dwError)
    //     {
    //         wprintf(L"IsUserInAdminGroup failed w/err %lu\n", dwError);
    //     }
    //
    static BOOL IsUserInAdminGroup();
    
    // 
    //   FUNCTION: IsRunAsAdmin()
    //
    //   PURPOSE: The function checks whether the current process is run as 
    //   administrator. In other words, it dictates whether the primary access 
    //   token of the process belongs to user account that is a member of the 
    //   local Administrators group and it is elevated.
    //
    //   RETURN VALUE: Returns TRUE if the primary access token of the process 
    //   belongs to user account that is a member of the local Administrators 
    //   group and it is elevated. Returns FALSE if the token does not.
    //
    //   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
    //   contains the Win32 error code of the failure.
    //
    //   EXAMPLE CALL:
    //     try 
    //     {
    //         if (IsRunAsAdmin())
    //             wprintf (L"Process is run as administrator\n");
    //         else
    //             wprintf (L"Process is not run as administrator\n");
    //     }
    //     catch (DWORD dwError)
    //     {
    //         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
    //     }
    //
    static BOOL IsRunAsAdmin();

    //
    //   FUNCTION: IsProcessElevated()
    //
    //   PURPOSE: The function gets the elevation information of the current 
    //   process. It dictates whether the process is elevated or not. Token 
    //   elevation is only available on Windows Vista and newer operating 
    //   systems, thus IsProcessElevated throws a C++ exception if it is called 
    //   on systems prior to Windows Vista. It is not appropriate to use this 
    //   function to determine whether a process is run as administartor.
    //
    //   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if 
    //   it is not.
    //
    //   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
    //   which contains the Win32 error code of the failure. For example, if 
    //   IsProcessElevated is called on systems prior to Windows Vista, the error 
    //   code will be ERROR_INVALID_PARAMETER.
    //
    //   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the 
    //   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
    //   TokenElevationTypeFull) of the process. It is different from 
    //   TokenElevation in that, when UAC is turned off, elevation type always 
    //   returns TokenElevationTypeDefault even though the process is elevated 
    //   (Integrity Level == High). In other words, it is not safe to say if the 
    //   process is elevated based on elevation type. Instead, we should use 
    //   TokenElevation.
    //
    //   EXAMPLE CALL:
    //     try 
    //     {
    //         if (IsProcessElevated())
    //             wprintf (L"Process is elevated\n");
    //         else
    //             wprintf (L"Process is not elevated\n");
    //     }
    //     catch (DWORD dwError)
    //     {
    //         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
    //     }
    //
    static BOOL IsProcessElevated();

    //
    //   FUNCTION: GetProcessIntegrityLevel()
    //
    //   PURPOSE: The function gets the integrity level of the current process. 
    //   Integrity level is only available on Windows Vista and newer operating 
    //   systems, thus GetProcessIntegrityLevel throws a C++ exception if it is 
    //   called on systems prior to Windows Vista.
    //
    //   RETURN VALUE: Returns the integrity level of the current process. It is 
    //   usually one of these values:
    //
    //     SECURITY_MANDATORY_UNTRUSTED_RID (SID: S-1-16-0x0)
    //     Means untrusted level. It is used by processes started by the 
    //     Anonymous group. Blocks most write access. 
    //
    //     SECURITY_MANDATORY_LOW_RID (SID: S-1-16-0x1000)
    //     Means low integrity level. It is used by Protected Mode Internet 
    //     Explorer. Blocks write acess to most objects (such as files and 
    //     registry keys) on the system. 
    //
    //     SECURITY_MANDATORY_MEDIUM_RID (SID: S-1-16-0x2000)
    //     Means medium integrity level. It is used by normal applications 
    //     being launched while UAC is enabled. 
    //
    //     SECURITY_MANDATORY_HIGH_RID (SID: S-1-16-0x3000)
    //     Means high integrity level. It is used by administrative applications 
    //     launched through elevation when UAC is enabled, or normal 
    //     applications if UAC is disabled and the user is an administrator. 
    //
    //     SECURITY_MANDATORY_SYSTEM_RID (SID: S-1-16-0x4000)
    //     Means system integrity level. It is used by services and other 
    //     system-level applications (such as Wininit, Winlogon, Smss, etc.)  
    //
    //   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
    //   which contains the Win32 error code of the failure. For example, if 
    //   GetProcessIntegrityLevel is called on systems prior to Windows Vista, 
    //   the error code will be ERROR_INVALID_PARAMETER.
    //
    //   EXAMPLE CALL:
    //     try 
    //     {
    //         DWORD dwIntegrityLevel = GetProcessIntegrityLevel();
    //     }
    //     catch (DWORD dwError)
    //     {
    //         wprintf(L"GetProcessIntegrityLevel failed w/err %lu\n", dwError);
    //     }
    //
    static DWORD GetProcessIntegrityLevel();
};

