// ftdi_fifo.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "FTD2XX.h"

FT_HANDLE ftHandle = 0;
bool aborted = false;

BOOL WINAPI ctrl_handler(DWORD CtrlType)
{
    switch (CtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        std::cout << "closing...";
        if (ftHandle) FT_Close(ftHandle);
        std::cout << "done\n";
        return FALSE;
    default:
        return FALSE;
    }
}

void on_recv_data(const uint8_t* data, DWORD len)
{
    static uint8_t last = 0;
    for (DWORD i = 0; i < len; i += 1)
    {
        if (last != data[i])
        {
            printf("%02X\n", data[i]);
            last = data[i];
        }
    }
}

int main()
{
    FT_STATUS ftStatus;
    DWORD numDevs;    

    if (!SetConsoleCtrlHandler(ctrl_handler, TRUE))
        printf("WARNING: ctrl handler not added\n");

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

    std::cout << "start receiving..." << std::endl;

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
}
