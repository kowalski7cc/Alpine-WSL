#define WIN32_LEAN_AND_MEAN
#define STRICT_GS_ENABLED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#include <windows.h>
#include <pathcch.h>
#include <wslapi.h>
#include <atlbase.h>
#include <cstdio>
#include <cstdlib>
#include <string>

#pragma comment(lib, "pathcch")

void show_usage();

const WCHAR DistributionName[] = L"Alpine";

int main(int args, char* argc[])
{
	SetConsoleTitleW(DistributionName);
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);
	HMODULE wslapi = LoadLibraryW(L"wslapi");
	if (!wslapi)
	{
		printf("Windows Subsystem for Linux isn't installed\n");
		printf("Press a key to exit\n");
		getchar();
		ExitProcess(EXIT_FAILURE);
	}
	auto WslIsDistributionRegisteredPtr = AtlGetProcAddressFn(wslapi, WslIsDistributionRegistered);
	auto WslLaunchInteractivePtr = AtlGetProcAddressFn(wslapi, WslLaunchInteractive);
	auto WslRegisterDistributionPtr = AtlGetProcAddressFn(wslapi, WslRegisterDistribution);
	auto WslUnregisterDistribution = AtlGetProcAddressFn(wslapi, WslUnregisterDistribution);

	if (!WslIsDistributionRegisteredPtr(DistributionName))
	{
		printf("Installing, this may take a few seconds...\n");
		WCHAR tar_ball[MAX_PATH];
		GetModuleFileNameW(nullptr, tar_ball, ARRAYSIZE(tar_ball));
		PathCchRemoveFileSpec(tar_ball, ARRAYSIZE(tar_ball));
		PathCchCombine(tar_ball, ARRAYSIZE(tar_ball), tar_ball, L"install.tar.gz");
		HRESULT last_error = WslRegisterDistributionPtr(DistributionName, tar_ball);
		if (FAILED(last_error))
		{
			printf("Installation failed: 0x%08lx", last_error);
			ExitProcess(EXIT_FAILURE);
		}
		printf("Installation successful!\n");
	}
	else if (args > 1)
	{
		if (strcmp(argc[1], "clean") == 0)
		{
			wprintf(L"This will remove this distro (%s) from the filesystem.\n", DistributionName);
			printf("Are you sure you would like to proceed? (This cannot be undone)\n");
			printf("Type \"y\" to continue: ");
			char yn;
			scanf_s("%c", &yn);
			fflush(stdin);
			if (yn == 'y')
			{
				HRESULT last_error = WslUnregisterDistribution(DistributionName);
				if (FAILED(last_error))
				{
					printf("Uninstall failed: 0x%08lx", last_error);
					ExitProcess(EXIT_FAILURE);
				}
				printf("Successfully removed distro.");
				ExitProcess(EXIT_SUCCESS);
			}

			fprintf(stderr, "Accepting is required to proceed. Exiting...");
			ExitProcess(EXIT_FAILURE);
		}
		else if (strcmp(argc[1], "help") == 0 || strcmp(argc[1], "-h") == 0 || strcmp(argc[1], "/?") == 0)
		{
			show_usage();
			ExitProcess(EXIT_SUCCESS);
		}
		else if (strcmp(argc[1], "run") == 0)
		{
			ULONG exit_code;
			HRESULT last_error = WslLaunchInteractivePtr(DistributionName, nullptr, TRUE, &exit_code);
			if (FAILED(last_error))
			{
				printf("Launch shell failed: 0x%08lx", last_error);
				ExitProcess(EXIT_FAILURE);
			}
			ExitProcess(exit_code);
		}
	}
	ULONG exit_code;
	HRESULT last_error = WslLaunchInteractivePtr(DistributionName, nullptr, FALSE, &exit_code);
	if (FAILED(last_error))
	{
		printf("Launch shell failed: 0x%08lx", last_error);
		ExitProcess(EXIT_FAILURE);
	}
	ExitProcess(exit_code);
}

void show_usage()
{
	wprintf(L"Useage :\n");
	wprintf(L"    <no args>\n");
	wprintf(L"      - Launches the distro's default behavior. By default, this launches your default shell.\n\n");
	wprintf(L"    run <command line>\n");
	wprintf(L"      - Run the given command line in that distro.\n\n");
	wprintf(L"    config [setting [value]]\n");
	wprintf(L"      - `--default-user <user>`: Set the default user for this distro to <user>\n");
	wprintf(L"      - `--default-uid <uid>`: Set the default user uid for this distro to <uid>\n");
	wprintf(L"      - `--append-path <on|off>`: Switch of Append Windows PATH to $PATH\n");
	wprintf(L"      - `--mount-drive <on|off>`: Switch of Mount drives\n\n");
	wprintf(L"    get [setting]\n");
	wprintf(L"      - `--default-uid`: Get the default user uid in this distro\n");
	wprintf(L"      - `--append-path`: Get on/off status of Append Windows PATH to $PATH\n");
	wprintf(L"      - `--mount-drive`: Get on/off status of Mount drives\n");
	wprintf(L"      - `--lxuid`: Get LxUID key for this distro\n\n");
	wprintf(L"    clean\n");
	wprintf(L"     - Uninstalls the distro.\n\n");
	wprintf(L"    help\n");
	wprintf(L"      - Print this usage message.\n\n");
}