#pragma once

extern "C" my_bool IsConnected_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
extern "C" void IsConnected_deinit(UDF_INIT *initid);
extern "C" longlong IsConnected(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

extern "C" my_bool Disconnect_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
extern "C" void Disconnect_deinit(UDF_INIT *initid);
extern "C" longlong Disconnect(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

extern "C" my_bool Connect_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
extern "C" void Connect_deinit(UDF_INIT *initid);
extern "C" longlong Connect(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

extern "C" my_bool SendUserMessage_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
extern "C" void SendUserMessage_deinit(UDF_INIT *initid);
extern "C" longlong SendUserMessage(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

extern "C" my_bool Notify_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
extern "C" void Notify_deinit(UDF_INIT *initid);
extern "C" longlong Notify(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);