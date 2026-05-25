#include <windows.h>
#include <iostream>

#define IOCTL_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x3000, METHOD_BUFFERED, GENERIC_READ | GENERIC_WRITE)

struct DeviceRequest
{
	USHORT a;
	USHORT b;
};

struct DeviceResponse
{
	USHORT result;
};
// ===========================

int main()
{
	HANDLE hDevice = CreateFileW(L"\\\\.\\Kernel_Bridge_Link",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DeviceRequest request = { 0 };
	request.a = 10;
	request.b = 15;

	LPVOID input = (LPVOID)&request;

	DeviceResponse response = { 0 };
	LPVOID answer = (LPVOID)&response;
	DWORD bytes = 0;

	bool res = DeviceIoControl(hDevice, IOCTL_CODE, input, sizeof(DeviceRequest),
		answer, sizeof(DeviceResponse), &bytes, NULL);

	response = *((DeviceResponse*)answer);
	std::cout << "Sum : " << response.result << std::endl;
	char ch;
	std::cin >> ch;

	CloseHandle(hDevice);
}
