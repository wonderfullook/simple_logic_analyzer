// ftdi_fifo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "FTD2XX.h"

FT_HANDLE ftHandle = 0;
bool aborted = false;
FILE* f_dump = NULL;

#include <windows.h>

const char *generate_file_name(void)
{
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    static char str[100];
    sprintf_s(str, "%02d-%02d-%02d_%02d_%02d.bin", wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond);
    //strcpy_s(str, "dump.bin");
    return str;
}

void clean_up(void)
{
    std::cout << "closing...";
    if (ftHandle)
    {
        FT_Close(ftHandle);
        ftHandle = NULL;
    }
    if (f_dump)
    {
        fclose(f_dump);
        f_dump = NULL;
    }
    std::cout << "done\n";
}

BOOL WINAPI ctrl_handler(DWORD CtrlType)
{
    switch (CtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        clean_up();
        return FALSE;
    default:
        return FALSE;
    }
}

void on_recv_data(const uint8_t* data, DWORD len)
{
    static uint8_t last = 0;
    fwrite(data, 1, len, f_dump);

    for (DWORD i = 0; i < len; i += 1)
    {
        if (last != data[i])
        {
            printf("%02X\r", data[i]);
            last = data[i];
        }
    }
}

int main(int argc, char** argv)
{
    FT_STATUS ftStatus;
    DWORD numDevs;

    if (!SetConsoleCtrlHandler(ctrl_handler, TRUE))
        printf("WARNING: ctrl handler not added\n");

    const char* fn = generate_file_name();
    if (fopen_s(&f_dump, fn, "wb") != 0)
        printf("WARNING: failed to open: %s\n", fn);

    ftStatus = FT_ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY);
    if ((ftStatus != FT_OK) || (numDevs < 1))
    {
        printf("no deivce!\n");
        return -1;
    }

    UCHAR Mask = 0xff;
    UCHAR Mode = 0x40; // = Single Channel Synchronous 245 FIFO Mode (FT2232H and FT232H devices only)

    ftStatus = FT_Open(0, &ftHandle);
    if (ftStatus != FT_OK) {
        printf("Error: FT_Open!\n");
        return -1;
    }

    ftStatus = FT_SetBitMode(ftHandle, Mask, Mode);
    if (ftStatus != FT_OK) {
        FT_Close(ftHandle);
        printf("FT_SetBitMode\n");
        return -2;
    }

    FT_SetTimeouts(ftHandle, 1000, 0);

    printf("start receiving (%s) ...\n", fn);

    while ((ftStatus == FT_OK) && !aborted)
    {
        DWORD BytesReceived = 0;
        uint8_t RxBuffer[256];
        DWORD RxBytes = sizeof(RxBuffer);

        ftStatus = FT_Read(ftHandle, RxBuffer, RxBytes, &BytesReceived);

        if (BytesReceived > 0) {
            on_recv_data(RxBuffer, BytesReceived);
        }
    }
    clean_up();
}
