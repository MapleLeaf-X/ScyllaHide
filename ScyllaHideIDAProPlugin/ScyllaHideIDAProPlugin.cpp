#define USE_STANDARD_FILE_FUNCTIONS
//#pragma warning(disable : 4996 4512 4127 4201)

//#define BUILD_IDA_64BIT

template <size_t x>
struct show_size;

//for 64bit - p64
#ifdef BUILD_IDA_64BIT
#define __EA64__
#pragma comment(lib, "x64_win_vc_64/ida.lib")
#else
//for 32bit - plw
#pragma comment(lib, "x64_win_vc_32/ida.lib")
#endif

#include <Windows.h>
#pragma warning(push, 4)
#pragma warning(disable: 4267 4244 4201)
#include <ida.hpp>
#include <idp.hpp>
#include <dbg.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#pragma warning(pop)
#include <Scylla/Logger.h>
#include <Scylla/Settings.h>
#include <Scylla/Version.h>
#include <Scylla/Util.h>
#include <Scylla/OsInfo.h>

#include "..\PluginGeneric\Injector.h"
#include "..\PluginGeneric\OptionsDialog.h"
#include "IdaServerClient.h"
#include "resource.h"

#include <locale>
#include <algorithm>
#include <functional>

typedef void(__cdecl* t_AttachProcess)(DWORD dwPID);

extern t_AttachProcess _AttachProcess;

const WCHAR g_scyllaHidex86DllFilename[] = L"HookLibraryx86.dll";
const WCHAR g_scyllaHidex86ServerFilename[] = L"ScyllaHideIDAServerx86.exe";
const WCHAR g_scyllaHidex86InjectorFilename[] = L"InjectorCLIx86";

#ifdef BUILD_IDA_64BIT
const WCHAR g_scyllaHidex64DllFilename[] = L"HookLibraryx64.dll";
const WCHAR g_scyllaHidex64ServerFilename[] = L"ScyllaHideIDAServerx64.exe";
const WCHAR g_scyllaHidex64InjectorFilename[] = L"InjectorCLIx64";
#endif

scl::Settings g_settings;
scl::Logger g_log;
std::wstring g_scyllaHideDllPath;
std::wstring g_scyllaHideIniPath;
std::wstring g_scyllaHideServerPath;
std::wstring g_scyllaHideInjectorPath;

HOOK_DLL_DATA g_hdd;

//globals
HINSTANCE hinst;
DWORD ProcessId = 0;
bool bHooked = false;
HMODULE hNtdllModule = 0;
PROCESS_INFORMATION ServerProcessInfo = {0};
STARTUPINFO ServerStartupInfo = {0};
bool isAttach = false;

static void LogCallback(const char* message) {
	msg("[%s] %s\n", SCYLLA_HIDE_NAME_A, message);
}

static void AttachProcess(DWORD dwPID) {
	int res = attach_process((pid_t)dwPID);

	switch(res) {
	case -1:
		MessageBoxA(nullptr, //(HWND)callui(ui_get_hwnd).vptr,
			"Can't attach to that process !",
			"ScyllaHide Plugin", MB_OK | MB_ICONERROR);
		break;
	case -2:
		MessageBoxA(nullptr, //(HWND)callui(ui_get_hwnd).vptr,
			"Can't find that PID !",
			"ScyllaHide Plugin", MB_OK | MB_ICONERROR);
		break;
	}
}

static bool SetDebugPrivileges() {
	::TOKEN_PRIVILEGES Debug_Privileges;
	bool retVal = false;

	if(LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &Debug_Privileges.Privileges[0].Luid)) {
		HANDLE hToken = 0;
		if(::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
			Debug_Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			Debug_Privileges.PrivilegeCount = 1;

			retVal = ::AdjustTokenPrivileges(hToken, FALSE, &Debug_Privileges, 0, nullptr, nullptr) != FALSE;

			::CloseHandle(hToken);
		}
	}

	return retVal;
}

//callback for various debug events
static ssize_t idaapi debug_mainloop(void* user_data, int notification_code, va_list va) {
	switch(notification_code) {
	case dbg_process_attach:
	{
		isAttach = true;
		break; //attaching not supported
	}
	case dbg_process_start:
	{
		isAttach = false;

		const debug_event_t* dbgEvent = va_arg(va, const debug_event_t*);

		ProcessId = dbgEvent->pid;
		bHooked = false;
		ZeroMemory(&g_hdd, sizeof(HOOK_DLL_DATA));

		if(dbg != nullptr) {
			//char text[1000];
			//wsprintfA(text, "dbg->id %d processor %s", dbg->id , dbg->processor);
			//MessageBoxA(0, text, text,0);
			// dbg->id DEBUGGER_ID_WINDBG -> 64bit and 32bit
			// dbg->id DEBUGGER_ID_X86_IA32_WIN32_USER -> 32bit

			if(dbg->is_remote()) {
				qstring hoststring;
				char host[MAX_PATH] = {0};
				char port[6] = {0};
				wcstombs(port, g_settings.opts().idaServerPort.c_str(), _countof(port));

				get_process_options(NULL, NULL, NULL, &hoststring, NULL, NULL);
				GetHost((char*)hoststring.c_str(), host);

				//msg("Host-String: %s\n", hoststring.c_str());
				//msg("Host: %s\n", host);

#ifdef BUILD_IDA_64BIT
				//autostart server if necessary
				if(g_settings.opts().idaAutoStartServer) {
					if(!scl::FileExistsW(g_scyllaHideServerPath.c_str())) {
						g_log.LogError(L"Cannot find server executable %s\n", g_scyllaHideServerPath.c_str());
					}

					DWORD dwRunningStatus = 0;
					if(ServerProcessInfo.hProcess) {
						GetExitCodeProcess(ServerProcessInfo.hProcess, &dwRunningStatus);
					}

					if(dwRunningStatus != STILL_ACTIVE) {
						if(ServerProcessInfo.hProcess) {
							CloseHandle(ServerProcessInfo.hProcess);
							CloseHandle(ServerProcessInfo.hThread);
						}

						ZeroMemory(&ServerStartupInfo, sizeof(ServerStartupInfo));
						ZeroMemory(&ServerProcessInfo, sizeof(ServerProcessInfo));

						WCHAR commandline[MAX_PATH * 2] = {0};
						wcscpy(commandline, g_scyllaHideServerPath.c_str());
						wcscat(commandline, L" ");
						wcscat(commandline, g_settings.opts().idaServerPort.c_str());
						ServerStartupInfo.cb = sizeof(ServerStartupInfo);
						if(!CreateProcessW(0, commandline, NULL, NULL, FALSE, 0, NULL, NULL, &ServerStartupInfo, &ServerProcessInfo)) {
							g_log.LogError(L"Cannot start server, error %d", GetLastError());
						}
						else {
							g_log.LogInfo(L"Started IDA Server successfully");
						}
					}
				}
#endif
				if(ConnectToServer(host, port)) {
					if(!SendEventToServer(notification_code, ProcessId)) {
						g_log.LogError(L"SendEventToServer failed");
					}
				}
				else {
					g_log.LogError(L"Cannot connect to host %s", host);
				}
			}
			else {
				if(!bHooked) {
#ifdef BUILD_IDA_64BIT
					auto hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessId);
					if(hProcess) {
						if(!scl::IsWow64Process(hProcess)) { // Only apply on native x64 OS, see dbg_library_unload below
							ReadNtApiInformation(&g_hdd);

							bHooked = true;
							startInjection(ProcessId, &g_hdd, g_scyllaHideDllPath.c_str(), true);
						}
						else {
							__debugbreak();
						}
						::CloseHandle(hProcess);
					}
#else
					/*bHooked = true;

					ZeroMemory(&ServerStartupInfo, sizeof(ServerStartupInfo));
					ZeroMemory(&ServerProcessInfo, sizeof(ServerProcessInfo));

					std::wostringstream oss;
					oss << g_scyllaHideInjectorPath << L" pid:" << ProcessId
						<< L" \"" << g_scyllaHideDllPath << L"\" nowait";
					WCHAR commandline[MAX_PATH * 2] = {0};
					wcscpy(commandline, oss.str().c_str());
					ServerStartupInfo.cb = sizeof(ServerStartupInfo);
					if(::CreateProcessW(0, commandline, NULL, NULL, FALSE, 0, NULL, NULL, &ServerStartupInfo, &ServerProcessInfo)) {
						::CloseHandle(ServerProcessInfo.hProcess);
						::CloseHandle(ServerProcessInfo.hThread);
					}*/
#endif
				}
			}
		}
	}
	break;

	case dbg_process_exit:
	{
		if(!isAttach && dbg->is_remote()) {
			if(!SendEventToServer(notification_code, ProcessId)) {
				g_log.LogError(L"SendEventToServer failed");
			}

			CloseServerSocket();
		}
		ProcessId = 0;
		bHooked = false;
	}
	break;

	case dbg_library_load:
	{

		if(!isAttach) {
			if(dbg->is_remote()) {
				if(!SendEventToServer(notification_code, ProcessId)) {
					g_log.LogError(L"SendEventToServer failed");
				}
			}
			else if(bHooked) {
#ifdef BUILD_IDA_64BIT
				auto hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessId);
				if(hProcess) {
					if(!scl::IsWow64Process(hProcess)) {
						startInjection(ProcessId, &g_hdd, g_scyllaHideDllPath.c_str(), false);
					}
					else {
						__debugbreak();
					}
					::CloseHandle(hProcess);
				}
#else
				//
#endif
			}
#ifndef BUILD_IDA_64BIT
			else {
				va_list v;
				va_copy(v, va);
				auto evt = va_arg(v, const debug_event_t*);
				auto& name = evt->modinfo().name;
				std::string tmp{&name.c_str()[name.rfind('\\') + 1]};
				std::transform(tmp.begin(), tmp.end(), tmp.begin(),
					std::bind(std::toupper<std::string::value_type>, std::placeholders::_1, std::locale()));
				if(tmp == "WOW64CPU.DLL") {
					ZeroMemory(&ServerStartupInfo, sizeof(ServerStartupInfo));
					ZeroMemory(&ServerProcessInfo, sizeof(ServerProcessInfo));

					std::wostringstream oss;
					oss << g_scyllaHideInjectorPath << L" pid:" << ProcessId
						<< L" \"" << g_scyllaHideDllPath << L"\" nowait";
					WCHAR commandline[MAX_PATH * 2] = {0};
					wcscpy(commandline, oss.str().c_str());
					ServerStartupInfo.cb = sizeof(ServerStartupInfo);
					if(::CreateProcessW(0, commandline, NULL, NULL, FALSE, 0, NULL, NULL, &ServerStartupInfo, &ServerProcessInfo)) {
						::CloseHandle(ServerProcessInfo.hProcess);
						::CloseHandle(ServerProcessInfo.hThread);
						bHooked = true;
					}
				}
			}
#endif
		}

	}
	break;

#ifndef BUILD_IDA_64BIT
	case dbg_library_unload:
	{
		__debugbreak();
		if(scl::IsWindows64() && !bHooked) {
			// Bogus unload event which is actually a load of a native x64 DLL (ntdll, wow64, wow64cpu, wow64win)
			ReadNtApiInformation(&g_hdd);

			bHooked = true;
			startInjection(ProcessId, &g_hdd, g_scyllaHideDllPath.c_str(), true);
		}
	}
	break;
#endif

	case dbg_bpt:
	{
		thid_t tid = va_arg(va, thid_t);
		ea_t breakpoint_ea = va_arg(va, ea_t);
		va_arg(va, int*);
	}
	break;

	case dbg_exception:
	{
		const debug_event_t* dbgEvent = va_arg(va, const debug_event_t*);

	}
	break;
	}

	return 0;
}

//cleanup on plugin unload
static void idaapi IDAP_term(void) {
	unhook_from_notification_point(HT_DBG, debug_mainloop, NULL);
}

//called when user clicks in plugin menu or presses hotkey
static bool idaapi IDAP_run(size_t arg) {
	DialogBox(hinst, MAKEINTRESOURCE(IDD_OPTIONS), nullptr /* (HWND)callui(ui_get_hwnd).vptr */, &OptionsDlgProc);
	return true;
}

//init the plugin
static int idaapi IDAP_init(void) {
	//ensure target is PE executable
	if(inf.filetype != f_PE) return PLUGIN_SKIP;

	//install hook for debug mainloop
	if(!hook_to_notification_point(HT_DBG, debug_mainloop, NULL)) {
		g_log.LogError(L"Error hooking notification point");
		return PLUGIN_SKIP;
	}

	msg("##################################################\n");
	msg("# " SCYLLA_HIDE_NAME_A " v" SCYLLA_HIDE_VERSION_STRING_A " Copyright 2014-" COMPILE_YEAR_A " Aguila / cypher #\n");
	msg("##################################################\n");

	bHooked = false;
	ProcessId = 0;
	ZeroMemory(&ServerStartupInfo, sizeof(ServerStartupInfo));
	ZeroMemory(&ServerProcessInfo, sizeof(ServerProcessInfo));

	return PLUGIN_KEEP;
}

// There isn't much use for these yet, but I set them anyway.
static char IDAP_comment[] = SCYLLA_HIDE_NAME_A " usermode Anti-Anti-Debug Plugin";
static char IDAP_help[] = SCYLLA_HIDE_NAME_A;

// The name of the plug-in displayed in the Edit->Plugins menu
static char IDAP_name[] = SCYLLA_HIDE_NAME_A;

// The hot-key the user can use to run your plug-in.
static char IDAP_hotkey[] = "Alt-X";

// The all-important exported PLUGIN object
idaman ida_module_data plugin_t PLUGIN =
{
	IDP_INTERFACE_VERSION,
	0,
	IDAP_init,
	IDAP_term,
	IDAP_run,
	IDAP_comment,
	IDAP_help,
	IDAP_name,
	IDAP_hotkey
};

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD dwReason, LPVOID lpReserved) {
	if(dwReason == DLL_PROCESS_ATTACH) {
		hinst = hInstDll;
		_AttachProcess = AttachProcess;
		hNtdllModule = ::GetModuleHandleW(L"ntdll.dll");

		auto wstrPath = scl::GetModuleFileNameW(hInstDll);
		wstrPath.erase(wstrPath.rfind(L'\\') + 1);
		wstrPath.append(L"ScyllaHide\\");

		g_scyllaHideDllPath = wstrPath + g_scyllaHidex86DllFilename;
		g_scyllaHideIniPath = wstrPath + scl::Settings::kFileName;
		g_scyllaHideServerPath = wstrPath + g_scyllaHidex86ServerFilename;
		g_scyllaHideInjectorPath = wstrPath + g_scyllaHidex86InjectorFilename;

		auto log_file = wstrPath + scl::Logger::kFileName;
		g_log.SetLogFile(log_file.c_str());
		g_log.SetLogCb(scl::Logger::Info, LogCallback);
		g_log.SetLogCb(scl::Logger::Error, LogCallback);

		g_settings.Load(g_scyllaHideIniPath.c_str());

		if(!SetDebugPrivileges()) {
			g_log.LogInfo(L"Failed to set debug privileges");
		}

		if(!StartWinsock()) {
			MessageBox(nullptr, TEXT("Failed to start Winsock!"), TEXT("Error"), MB_ICONERROR);
		}
	}

	return TRUE;
}
