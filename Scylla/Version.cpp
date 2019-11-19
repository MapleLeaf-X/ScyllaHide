#include "Version.h"

void scl::ShowAboutBox(HWND hWnd) {
	::MessageBoxA(hWnd,
		SCYLLA_HIDE_NAME_A " Plugin v" SCYLLA_HIDE_VERSION_STRING_A " (" TEXT(__DATE__) ")\n\n"
		"Copyright (C) 2014-" COMPILE_YEAR_A " Aguila / cypher\n\n"
		"Special thanks to:\n"
		"- What for his POISON assembler source code\n"
		"- waliedassar for his blog posts\n"
		"- Peter Ferrie for his Anti-Debug PDFs\n"
		"- MaRKuS-DJM for OllyAdvanced assembler source code\n"
		"- Steve Micallef for his IDA SDK doc\n"
		"- Authors of PhantOm and StrongOD\n"
		"- Tuts4You, Exetools, Exelab community for testing\n"
		"- last but not least deepzero & mr.exodia for tech chats",
		SCYLLA_HIDE_NAME_A " Plugin", MB_ICONINFORMATION);
}
