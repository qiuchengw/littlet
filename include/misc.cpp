#include <winsock2.h>
#include <windows.h>

#include "misc.h"
#include <Iphlpapi.h>
#include "deps/zmq.hpp"

#pragma comment(lib, "Iphlpapi.lib")

namespace littlet
{
    bool SendWebRequest(const CStdString& name, const CStdString& content)
    {
        // 使用zmq的request
        zmq::context_t ctx;
        zmq::socket_t sock(ctx, ZMQ_REQ);

        // 远程地址
        // sock.connect("tcp://www.0woow.com:5230");
        try
        {
            sock.connect("tcp://127.0.0.1:5231");

            // 第一帧是名字地址
            sock.send(name.c_str(), name.GetLength() * sizeof(CStdString::value_type), ZMQ_SNDMORE);

            // 第二帧是mac地址
            CStdString mac_addr;
            GetMACAddress(mac_addr);
            sock.send(mac_addr.c_str(), mac_addr.GetLength() * sizeof(CStdString::value_type), ZMQ_SNDMORE);

            // 最后一个是内容
            sock.send(content.c_str(), content.GetLength() * sizeof(CStdString::value_type));

            // 最后必然有一个接收的动作
            char buf[64];
            sock.recv(buf, 64);

            sock.close();
            ctx.close();

        }
        catch (zmq::error_t& t)
        {
#ifdef _DEBUG
            OutputDebugStringA(t.what());
            OutputDebugStringA("\n");
#endif
            return false;
        }

        return true;
    }

    bool GetMACAddress(CStdString& mac_addr)
    {
        // Get the buffer length required for IP_ADAPTER_INFO.
        ULONG BufferLength = 0;
        BYTE* pBuffer = 0;
        if( ERROR_BUFFER_OVERFLOW == GetAdaptersInfo( 0, &BufferLength ))
        {
            // Now the BufferLength contain the required buffer length.
            // Allocate necessary buffer.
            pBuffer = new BYTE[ BufferLength ];
        }
        else
        {
            return false;
        }

        // Get the Adapter Information.
        PIP_ADAPTER_INFO pAdapterInfo =
            reinterpret_cast<PIP_ADAPTER_INFO>(pBuffer);
        GetAdaptersInfo( pAdapterInfo, &BufferLength );

        // Iterate the network adapters and print their MAC address.
        // while( pAdapterInfo )
        bool bok = false;
        if (nullptr != pAdapterInfo)
        {
            // Assuming pAdapterInfo->AddressLength is 6.
            mac_addr.Format(_T("%02x:%02x:%02x:%02x:%02x:%02x"),
                pAdapterInfo->Address[0],
                pAdapterInfo->Address[1],
                pAdapterInfo->Address[2],
                pAdapterInfo->Address[3],
                pAdapterInfo->Address[4],
                pAdapterInfo->Address[5]);

            bok = true;
        }

        // deallocate the buffer.
        delete[] pBuffer;

        return bok;
    }


}