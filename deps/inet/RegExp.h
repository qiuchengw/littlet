/**
 *  Copyright 2008-2009 Cheng Shi.  All rights reserved.
 *  Email: shicheng107@hotmail.com
 */

#ifndef REGEXP_H
#define REGEXP_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#pragma warning(push)
#pragma warning(disable: 6385 6011 4127 4018)
#include "atlrx.h"

/*
 * Parameters
 *  [in] regExp: Value of type string which is the input regular expression.
 *  [in] caseSensitive: Value of type bool which indicate whether the parse is case sensitive.
 *  [in] groupCount: Value of type int which is the group count of the regular expression.
 *  [in] source: Value of type string reference which is the source to parse.
 *  [out] result: Value of type vecotr of strings which is the output of the parse.
 *  [in] allowDuplicate: Value of type bool which indicates whether duplicate items are added to the output result.
 *
 * Return Value
 *  Returns true if the function succeeds, or false otherwise.
 *
 * Remarks
 *  The output result is devided into groups.  User should get the groups according to the group count.  For example:
 *  1. RegExp = L"{ab}", source = L"abcabe", then result = L"ab", L"ab".
 *  2. RegExp = L"{ab}{cd}", source = L"abcdeabecd", then result = L"ab", L"cd", L"ab", L"cd".
*/
inline bool ParseRegExp(const wstring &regExp, bool bCaseSensitive, 
	int nGroupCount, const wstring &source, vector<wstring> &result, bool allowDuplicate = false)
{
	if (regExp.empty() || source.empty() || (nGroupCount <= 0))
		return false;

	result.clear();
	CAtlRegExp<> re;
	if (re.Parse(regExp.c_str(), bCaseSensitive) != REPARSE_ERROR_OK)
		return FALSE;

	CAtlREMatchContext<> mc;
	if (!re.Match( source.c_str(), &mc ))
		return FALSE;

	const CAtlREMatchContext<>::RECHAR* szStart = 0;
	const CAtlREMatchContext<>::RECHAR* szEnd = 0;
	for (UINT iGroup = 0; (iGroup < mc.m_uNumGroups) && (iGroup < nGroupCount); ++iGroup)
	{
		mc.GetMatch(iGroup, &szStart, &szEnd);
		result.push_back(wstring(szStart,szEnd - szStart));
	}

	if (!allowDuplicate)
	{
		std::sort(result.begin(),result.end());
		auto itr = std::unique(result.begin(),result.end());
		result.erase(itr,result.end());
	}

    return true;
}

inline BOOL ParseRegExp(LPCWSTR pszRegExp, BOOL bCaseSensitive, 
	int nGroupCount, LPCWSTR pszSource, vector<CStdString> &result, 
	BOOL bAllowDuplicate = false)
{
	if (!wcslen(pszRegExp) || !wcslen(pszSource) || (nGroupCount <= 0))
		return FALSE;
	
	result.clear();
	CAtlRegExp<> re;
	if (re.Parse(pszRegExp, bCaseSensitive) != REPARSE_ERROR_OK)
		return FALSE;

	CAtlREMatchContext<> mc;
	if (!re.Match( pszSource, &mc ))
		return FALSE;

	const CAtlREMatchContext<>::RECHAR* szStart = 0;
	const CAtlREMatchContext<>::RECHAR* szEnd = 0;
	for (UINT iGroup = 0; (iGroup < mc.m_uNumGroups) && (iGroup < nGroupCount); ++iGroup)
	{
		mc.GetMatch(iGroup, &szStart, &szEnd);
		result.push_back(CStdString(szStart,szEnd - szStart));
	}

	if (!bAllowDuplicate)
	{
		std::sort(result.begin(),result.end());
		auto itr = std::unique(result.begin(),result.end());
		result.erase(itr,result.end());
	}

	return TRUE;
}

#pragma warning(pop)

#endif // REGEXP_H
