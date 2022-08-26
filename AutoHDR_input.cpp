#include "Windows.h"
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <conio.h>
#include <stdio.h>

void HDR()
{
	uint32_t pathCount, modeCount;

	uint8_t set[] = { 0x0A, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x14, 0x81, 0x00, 0x00,
					 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };

	uint8_t request[] = { 0x09, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x7C, 0x6F, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0xDB, 0x00,
						 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00 };

	if (ERROR_SUCCESS == GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount))
	{
		DISPLAYCONFIG_PATH_INFO* pathsArray = nullptr;
		DISPLAYCONFIG_MODE_INFO* modesArray = nullptr;

		const size_t sizePathsArray = pathCount * sizeof(DISPLAYCONFIG_PATH_INFO);
		const size_t sizeModesArray = modeCount * sizeof(DISPLAYCONFIG_MODE_INFO);

		pathsArray = static_cast<DISPLAYCONFIG_PATH_INFO*>(std::malloc(sizePathsArray));
		modesArray = static_cast<DISPLAYCONFIG_MODE_INFO*>(std::malloc(sizeModesArray));

		if (pathsArray != nullptr && modesArray != nullptr)
		{
			std::memset(pathsArray, 0, sizePathsArray);
			std::memset(modesArray, 0, sizeModesArray);

			if (ERROR_SUCCESS == QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, pathsArray,
				&modeCount, modesArray, 0))
			{
				DISPLAYCONFIG_DEVICE_INFO_HEADER* setPacket =
					reinterpret_cast<DISPLAYCONFIG_DEVICE_INFO_HEADER*>(set);
				DISPLAYCONFIG_DEVICE_INFO_HEADER* requestPacket =
					reinterpret_cast<DISPLAYCONFIG_DEVICE_INFO_HEADER*>(request);

				for (int i = 0; i < modeCount; i++)
				{
					if (modesArray[i].infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET)
					{
						setPacket->adapterId.HighPart = modesArray[i].adapterId.HighPart;
						setPacket->adapterId.LowPart = modesArray[i].adapterId.LowPart;
						setPacket->id = modesArray[i].id;

						requestPacket->adapterId.HighPart = modesArray[i].adapterId.HighPart;
						requestPacket->adapterId.LowPart = modesArray[i].adapterId.LowPart;
						requestPacket->id = modesArray[i].id;
					}
				}

				if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(requestPacket))
				{
					if (request[20] == 0xD1) // HDR is OFF
					{
						set[20] = 1;
						DisplayConfigSetDeviceInfo(setPacket);
					}
					else if (request[20] == 0xD3) // HDR is ON
					{
						set[20] = 0;
						DisplayConfigSetDeviceInfo(setPacket);
					}
				}
			}
			std::free(pathsArray);
			std::free(modesArray);
		}
	}
}

int main()
{

	char * pCmd = ::GetCommandLine();
	// skip the executable 
	if (*pCmd++ == L'"')
	{
		while (*pCmd++ != L'"');
	}
	else
	{
		while (*pCmd != NULL && *pCmd != L' ')
			++pCmd;
	}

	while (*pCmd == L' ')
		pCmd++;


	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	HDR();

	CreateProcess(NULL, pCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);


	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	HDR();

	return 0;

}
