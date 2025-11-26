#pragma once

#include <windows.h>

#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>

#define debugConsule


void RedirectIOToConsole() {
	// 1. Allocate a new console if not already open.
	if (::AllocConsole()) {

		// 2. Redirect C-style I/O using freopen_s.
		// This is the correct and standard way to redirect C streams.
		FILE* p_stdin = nullptr;
		FILE* p_stdout = nullptr;
		FILE* p_stderr = nullptr;

		// Redirect stdin
		freopen_s(&p_stdin, "CONIN$", "r", stdin);

		// Redirect stdout
		freopen_s(&p_stdout, "CONOUT$", "w", stdout);

		// Redirect stderr
		freopen_s(&p_stderr, "CONOUT$", "w", stderr);

		// 3. Clear C++ stream flags.
		// This resets the state of the C++ streams before we re-link them.
		std::cout.clear();
		std::cin.clear();
		std::cerr.clear();

		// 4. Re-link C++ streams to C streams by enabling synchronization.
		// This is the crucial step. By setting sync_with_stdio to true,
		// we tell the C++ streams to use the underlying C streams that we just redirected.
		std::ios_base::sync_with_stdio(true);
	}
}

void DeallocateMyConsole() {
	std::cout.flush();
	std::cout.clear();
	std::cerr.flush();
	std::cerr.clear();	
	std::cin.clear();

	if (FreeConsole()) {
		std::cout << "Console freed successfully." << std::endl;
	}
	else {
		std::cout << "Failed to free console." << std::endl;
	}
}
