#include "Version.h"

void scl::ShowAboutBox(HWND hWnd) {
	MessageBox(hWnd,
		SCYLLA_HIDE_NAME STR(" Plugin v") SCYLLA_HIDE_VERSION_STRING
		STR(" (") TEXT(__DATE__) STR(")\n\n")
		STR("Copyright (C) 2014-") COMPILE_YEAR STR(" Aguila / cypher\n\n")
		STR("Special thanks to:\n"
		"- What for his POISON assembler source code\n"
		"- waliedassar for his blog posts\n"
		"- Peter Ferrie for his Anti-Debug PDFs\n"
		"- MaRKuS-DJM for OllyAdvanced assembler source code\n"
		"- Steve Micallef for his IDA SDK doc\n"
		"- Authors of PhantOm and StrongOD\n"
		"- Tuts4You, Exetools, Exelab community for testing\n"
		"- last but not least deepzero & mr.exodia for tech chats"),
		SCYLLA_HIDE_NAME STR(" Plugin"), MB_ICONINFORMATION);
}
