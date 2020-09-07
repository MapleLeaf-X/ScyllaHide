#include "Version.h"

void scl::ShowAboutBox(HWND hWnd) {
	MessageBox(hWnd,
		SCYLLA_HIDE_NAME TEXT(" Plugin v") SCYLLA_HIDE_VERSION_STRING
		TEXT(" (") TEXT(__DATE__) TEXT(")\n\n")
		TEXT("Copyright (C) 2014-") COMPILE_YEAR TEXT(" Aguila / cypher\n\n")
		TEXT("Special thanks to:\n")
		TEXT("- What for his POISON assembler source code\n")
		TEXT("- waliedassar for his blog posts\n")
		TEXT("- Peter Ferrie for his Anti-Debug PDFs\n")
		TEXT("- MaRKuS-DJM for OllyAdvanced assembler source code\n")
		TEXT("- Steve Micallef for his IDA SDK doc\n")
		TEXT("- Authors of PhantOm and StrongOD\n")
		TEXT("- Tuts4You, Exetools, Exelab community for testing\n")
		TEXT("- last but not least deepzero & mr.exodia for tech chats"),
		SCYLLA_HIDE_NAME TEXT(" Plugin"), MB_ICONINFORMATION);
}
