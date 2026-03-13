///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016-2021, Intel Corporation
//
// SPDX-License-Identifier: MIT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Author(s):  Filip Strugar (filip.strugar@intel.com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "HFileSystem.h"
//#include "Core/HMemory.h"
//#include "System/HMemoryStream.h"
#include <fstream>
//#include "Core/HLog.h"
//#include "Core/HStringTools.h"

#include <HCore/Include/Core/HLog.h>
#include <HCore/Include/Core/HStringTools.h>
//#include "Platform/NativeFileDialog/nfd.hpp"

#include <stdio.h>
#include <fstream>
#include <sstream>

#pragma warning ( disable: 4996 )

namespace fs = std::filesystem;

using namespace Horizon;

fsPath HFileSystem::RelativePath(const fsPath& path, const fsPath& base)
{
	return std::filesystem::relative(path, base);
}

bool HFileSystem::CreateNullFile(const fsPath& path)
{
	std::fstream file;
	file.open(path, std::ios_base::out | std::ios_base::binary);
	file.close();
	return true;
}

fsPath HFileSystem::CleanupPath(const fsPath& inputPath, bool convertToLowercase, bool useBackslash)
{
	wstring ret = inputPath;
	if (convertToLowercase)
		ret = HStringTools::ToLowerCopy(ret);

	wstring::size_type foundPos;
	while ((foundPos = ret.find(L"/")) != wstring::npos)
		ret.replace(foundPos, 1, L"\\");
	while ((foundPos = ret.find(L"\\\\")) != wstring::npos)
		ret.replace(foundPos, 2, L"\\");

	// restore network path
	if ((ret.length() > 0) && (ret[0] == '\\'))
		ret = L"\\" + ret;

	// remove relative '..\' ("A\B\..\C" -> "A\C")
	std::vector<wstring> parts; foundPos = 0; wstring::size_type prevSep = 0;
	while ((foundPos = ret.find(L"\\", foundPos)) != wstring::npos)
	{
		foundPos++;
		wstring thisPart = ret.substr(prevSep, foundPos - prevSep);
		if (parts.size() > 0 && thisPart == L"..\\")
			parts.pop_back();
		else
			parts.push_back(thisPart);
		prevSep = foundPos;
	}
	parts.push_back(ret.substr(prevSep));

	ret = L"";
	for (const auto& part : parts)
		ret += part;

	if (!useBackslash)
		while ((foundPos = ret.find(L"\\")) != wstring::npos)
			ret.replace(foundPos, 1, L"/");

	return ret;
}

std::vector<fsPath> HFileSystem::FindExtension(const fsPath& startDirectory, const std::string& extName,
	bool recursive)
{
	std::vector<fsPath> result;

	for (auto& entry : std::filesystem::directory_iterator(startDirectory))
	{
		if (entry.path().filename().extension() == extName)
			result.emplace_back(entry.path());
	}

	return result;
}

fsPath HFileSystem::OpenFileDialog(const fsPath& initialDir, std::vector<HFileFilterItem> filters)
{
	fsPath path;

//	std::vector<nfdnfilteritem_t> nfdfilter;
//	for (auto& item : filters)
//	{
//		nfdfilter.push_back({ item.Name.c_str(), item.Spec.c_str() });
//	}
//	//NFD_PickFolderN()
//	nfdnchar_t* outPath;
//#ifdef PLATFORM_WIN32
//	nfdresult_t result = NFD_OpenDialogN(&outPath, nfdfilter.data(), static_cast<nfdfiltersize_t>(nfdfilter.size()), initialDir.c_str());
//#else
//	nfdresult_t result = NFD_OpenDialogN(&outPath, nfdfilter.data(), static_cast<nfdfiltersize_t>(nfdfilter.size()), initialDir.string().c_str());
//#endif
//
//	if (result == NFD_OKAY)
//	{
//		puts("Success!");
//
//		path = outPath;
//
//		NFD_FreePathN(outPath);
//
//		return path;
//	}

	return path;
}

fsPath HFileSystem::SaveFileDialog(const fsPath& defaultName, const fsPath& defaultPath, std::vector<HFileFilterItem> filters)
{
	fsPath path;

//	std::vector<nfdnfilteritem_t> nfdfilter;
//	for (auto& item : filters)
//	{
//		nfdfilter.push_back({ item.Name.c_str(), item.Spec.c_str() });
//	}
//
//	nfdnchar_t* outPath;
//#ifdef PLATFORM_WIN32
//	nfdresult_t result = NFD_SaveDialogN(&outPath, nfdfilter.data(), static_cast<nfdfiltersize_t>(nfdfilter.size()), defaultPath.c_str(), defaultName.c_str());
//#else
//	nfdresult_t result = NFD_SaveDialogN(&outPath, nfdfilter.data(), static_cast<nfdfiltersize_t>(nfdfilter.size()), defaultPath.string().c_str(), defaultName.string().c_str());
//#endif
//
//	if (result == NFD_OKAY)
//	{
//		puts("Success!");
//
//		path = outPath;
//
//		NFD_FreePathN(outPath);
//
//		return path;
//	}

	return path;
}

fsPath HFileSystem::SelectFolderDialog(const fsPath& initialDir)
{
	fsPath path;

//	nfdnchar_t* outPath;
//
//#ifdef PLATFORM_WIN32
//	nfdresult_t result = NFD_PickFolderN(&outPath, initialDir.c_str());
//#else
//	nfdresult_t result = NFD_PickFolderN(&outPath, initialDir.string().c_str());
//#endif
//
//	if (result == NFD_OKAY)
//	{
//		puts("Success!");
//
//		path = outPath;
//
//		NFD_FreePathN(outPath);
//
//		return path;
//	}

	return path;
}

#ifdef _WIN32
#pragma comment(lib,"Shell32.lib")
#include <shlobj_core.h>
#include <windows.h>
#include <tchar.h>
#include <shobjidl.h>
#endif

void HFileSystem::OpenSystemExplorerFolder(const fsPath& folderPath)
{
#ifdef _WIN32
	{
		auto path = CleanupPath(folderPath, true);
		LPCWSTR pszPathToOpen = path.c_str();
		//LPCWSTR pszPathToOpen = L"C:\\Windows";
		PIDLIST_ABSOLUTE pidl;
		if (SUCCEEDED(SHParseDisplayName(pszPathToOpen, 0, &pidl, 0, 0)))
		{
			// we don't want to actually select anything in the folder, so we pass an empty
			// PIDL in the array. if you want to select one or more items in the opened
			// folder you'd need to build the PIDL array appropriately
			ITEMIDLIST idNull = { 0 };
			LPCITEMIDLIST pidlNull[1] = { &idNull };
			SHOpenFolderAndSelectItems(pidl, 1, pidlNull, 0);
			ILFree(pidl);
		}
	}

#else
	// 非Windows平台，先将wstring转换为string
	std::string narrowPath = folderPath.string();
#if defined(__APPLE__)
	// macOS平台
	std::stringstream appleCmd;
	appleCmd << "open " << narrowPath;
	int appleRet = system(appleCmd.str().c_str());
	if (appleRet == -1) {
		std::cerr << "在macOS上打开文件夹出错。" << std::endl;
	}
#elif defined(__linux__)
	// Linux平台
	std::stringstream linuxCmd;
	linuxCmd << "xdg - open " << narrowPath;
	int linuxRet = system(linuxCmd.str().c_str());
	if (linuxRet == -1) {
		std::cerr << "在Linux上打开文件夹出错。" << std::endl;
	}
#else
	std::cerr << "不支持的平台。" << std::endl;
#endif
#endif
}

bool HFileSystem::ReadTextFromFile(const fsPath& path, std::string& text)
{
	std::ifstream file(path, std::ios::in | std::ios::binary); // 使用二进制模式避免换行符自动转换
	if (!file.is_open())
		return false;

	std::ostringstream ss;
	ss << file.rdbuf();  // 读取整个文件内容到 stringstream
	text = ss.str();     // 获取字符串
	return true;
}

bool HFileSystem::WriteTextToFile(const fsPath& path, const std::string& text)
{
    std::ofstream file(path, std::ios::out | std::ios::binary);

    if (!file.is_open())
        return false;

    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    return file.good();
}

void HFileSystem::SplitPath(const fsPath& inFullPath, string* outDirectory, string* outFileName, string* outFileExt)
{
	if (outDirectory != nullptr) 
		*outDirectory = inFullPath.parent_path().string();

	if (outFileName != nullptr)  
		*outFileName = inFullPath.stem().string();

	if (outFileExt != nullptr)   
		*outFileExt = inFullPath.extension().string();
}

void HFileSystem::SplitPath(const fsPath& inFullPath, wstring* outDirectory, wstring* outFileName, wstring* outFileExt)
{
	if (outDirectory != nullptr)
		*outDirectory = inFullPath.parent_path().wstring();

	if (outFileName != nullptr)
		*outFileName = inFullPath.stem().wstring();

	if (outFileExt != nullptr)
		*outFileExt = inFullPath.extension().wstring();
}

void HFileSystem::SplitPath(const fsPath& inFullPath, string& outDirectory, string& outFileName, string& outFileExt)
{
	outDirectory = inFullPath.parent_path().string();
	outFileName = inFullPath.stem().string();
	outFileExt = inFullPath.extension().string();
}

void HFileSystem::SplitPath(const fsPath& inFullPath, wstring& outDirectory, wstring& outFileName, wstring& outFileExt)
{
	outDirectory = inFullPath.parent_path().wstring();
	outFileName = inFullPath.stem().wstring();
	outFileExt = inFullPath.extension().wstring();
}

string HFileSystem::SplitPathExt(const fsPath& inFullPath)
{
	string ret;
	SplitPath(inFullPath, nullptr, nullptr, &ret);
	return ret;
}

std::string HFileSystem::ToWindowsPath(const std::string& path)
{
	std::string result = path;
	for (auto& ch : result)
	{
		if (ch == '/')
			ch = '\\';
	}
	return result;
}

std::string HFileSystem::ToUnixPath(const std::string& path)
{
	std::string result = path;
	for (auto& ch : result)
	{
		if (ch == '\\')
			ch = '/';
	}
	return result;
}

bool HFileSystem::IsSubPath(const fsPath& base, const fsPath& target)
{
	auto absBase = std::filesystem::weakly_canonical(base);
	auto absTarget = std::filesystem::weakly_canonical(target);

	// 如果 target 相对于 base 的路径没有 ".."，说明 target 在 base 里面
	auto rel = std::filesystem::relative(absTarget, absBase);
	for (auto& part : rel) {
		if (part == "..")
			return false;
	}
	return rel != absTarget; // rel == absTarget 表示不在 base 里
}

std::string HFileSystem::GetFileNameFromPath(const fsPath& path)
{
	return path.filename().string();
}

bool HFileSystem::PathHasDirectory(const string& inFullPath)
{
	string dir;
	SplitPath(inFullPath, &dir, nullptr, nullptr);
	return dir != "";
}

wstring HFileSystem::FindLocalFile(const wstring& fileName)
{
	if (ExistsFile(HCore::GetWorkingDirectory() + fileName))
		return HFileSystem::CleanupPath(HCore::GetWorkingDirectory() + fileName, false);
	if (ExistsFile(HCore::GetExecutableDirectory() + fileName))
		return HFileSystem::CleanupPath(HCore::GetExecutableDirectory() + fileName, false);
	if (ExistsFile(fileName.c_str()))
		return HFileSystem::CleanupPath(fileName, false);

	return L"";
}

//wstring HFileSystem::FixExtension(const wstring& path, const wstring& _ext)
//{
//	string ext = _ext;
//	if (ext.length() == 0) return path;
//	if (ext[0] != L'.')
//	{
//		assert(false);
//		return L"";
//	}
//
//	string currentExt;
//	SplitPath(path, nullptr, nullptr, &currentExt);
//	if (HStringTools::CompareNoCase(currentExt, ext) != 0)
//		return path + ext;
//	else
//		return path;
//}

string HFileSystem::FixExtension(const string& path, const string& _ext)
{
	string ext = _ext;
	if (ext.length() == 0) return path;
	if (ext[0] != L'.')
	{
		assert(false);
		return "";
	}
	string currentExt;
	SplitPath(path, nullptr, nullptr, &currentExt);
	if (HStringTools::CompareNoCase(currentExt, ext) != 0)
		return path + ext;
	else
		return path;
}

//string HFileSystem::SelectFolderDialog(const string& initialDir)
//{
//	return HStringTools::SimpleNarrow(SelectFolderDialog(HStringTools::SimpleWiden(initialDir)));
//}
//
//void HFileSystem::OpenSystemExplorerFolder(const string& folderPath)
//{
//	OpenSystemExplorerFolder(HStringTools::SimpleWiden(folderPath));
//}