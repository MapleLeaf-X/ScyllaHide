#pragma once

bool StartWinsock();
bool ConnectToServer(const char * host, const char * port);
bool SendEventToServer(unsigned long notifcation_code, unsigned long ProcessId);
void CloseServerSocket();
bool GetHost(char * input, char * output);
bool SendSettingsToServer(unsigned long ProcessId);
bool SendInjectToServer(unsigned long ProcessId);
