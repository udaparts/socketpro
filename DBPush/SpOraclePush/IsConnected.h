#pragma once

extern "C" int IsConnected();
extern "C" int Disconnect();
extern "C" int Connect(char *strConnectionString);
extern "C" int SendUserMessage(char *strMsg, char *strUserId);
extern "C" int Notify(char *strMsg, char *strGroups);