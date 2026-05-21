#include <windows.h>
#include <iostream>

void LaunchEditor()
{
	STARTUPINFOA si{};
	PROCESS_INFORMATION pi{};

	si.cb = sizeof(si);

	const char* exe =
		"NevernessEditor.exe";

	BOOL result = CreateProcessA(
		exe,
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	);

	if (!result)
	{
		std::cout << "Failed to launch editor\n";
	}
}

int main()
{
	LaunchEditor();

	// native engine loop
	while (true)
	{
		Sleep(16);
	}

	return 0;
}