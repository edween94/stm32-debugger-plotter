#include "STM32Detector.h"
#include <unordered_map>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdio>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
#endif

#ifdef _WIN32
    using SocketType = SOCKET;
    constexpr SocketType InvalidSocket = INVALID_SOCKET;
    constexpr int SocketError = SOCKET_ERROR;
#else
    using SocketType = int;
    constexpr SocketType InvalidSocket = -1;
    constexpr int SocketError = -1;
    #define closesocket close
#endif

static const uint32_t IDCODE_ADDRS[] = {
    0xE0042000,
    0x40015800,
    0x5C001000,
    0xE0044000
};

static bool socketInit()
{
    #ifdef _WIN32
        WSADATA wsaData;
        return (WSAStartup(MAKEWORD(2,2), &wsaData) == 0);
    #else
        return true;
    #endif
}

static void socketCleanup()
{
    #ifdef _WIN32
        WSACleanup();
    #endif
}

static std::string getLastError()
{
    #ifdef _WIN32
        int err = WSAGetLastError();
        char buf[256];
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, err, 0, buf, sizeof(buf), nullptr);
        return std::string(buf);
    #else
        return std::string(strerror(errno));
    #endif
}

static bool sendCommand(SocketType skt, const char* cmd, char* resp, int respSize)
{
    std::string fullCmd = std::string(cmd) + "\r\n";
    int sent = send(skt, fullCmd.c_str(), (int)fullCmd.size(), 0);
    if (sent == SocketError)
    {
        printf("DEBUG: send() failed: %s\n", getLastError().c_str());
        return false;
    }

    memset(resp, 0, respSize);
    int total = 0;

    for (int i = 0; i < 20; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        int received = recv(skt, resp + total, respSize - total - 1, 0);
        if (received > 0) total += received;
        else if (received == SocketError)
        {
            printf("DEBUG: recv() error: %s\n", getLastError().c_str());
            break;
        }

        if (total <= 0) continue;

        // Filter some telnet negotiation IAC sequences (0xFF ...)
        std::string temp(resp, resp + total);
        std::string filtered;
        for (size_t k = 0; k < temp.size(); ++k)
        {
            unsigned char c = static_cast<unsigned char>(temp[k]);
            if (c == 0xFF)
            {
                // skip simple IAC sequences (this is a best-effort filter)
                if (k + 2 < temp.size()) k += 2;
                continue;
            }
            filtered.push_back(static_cast<char>(c));
        }

        // copy filtered back into resp buffer
        strncpy(resp, filtered.c_str(), respSize - 1);

        // consider response complete if we see an address/value pair, or the OpenOCD prompt
        if (filtered.find(": ") != std::string::npos) return true;
        if (filtered.find("\n>") != std::string::npos) return true;
        if (!filtered.empty() && (filtered.back() == '>')) return true;
    }

    return false;
}

static uint32_t parseID(const char* resp)
{
    const char* colon = strchr(resp, ':');
    if (!colon) return 0;

    uint32_t value = 0;
    if (sscanf(colon + 1, "%x", &value) == 1) return value;

    return 0;
}

const char* getSTM32Config(uint16_t d_ID)
{
    static const std::unordered_map<uint16_t, const char*> STM32_CONFIGS
    {
        {0x440, "stm32f0x.cfg"}, {0x442, "stm32f0x.cfg"}, {0x444, "stm32f0x.cfg"}, {0x445, "stm32f0x.cfg"}, {0x448, "stm32f0x.cfg"},
        {0x410, "stm32f1x.cfg"}, {0x412, "stm32f1x.cfg"}, {0x414, "stm32f1x.cfg"}, {0x418, "stm32f1x.cfg"}, {0x420, "stm32f1x.cfg"}, {0x428, "stm32f1x.cfg"}, {0x430, "stm32f1x.cfg"},
        {0x411, "stm32f2x.cfg"},
        {0x422, "stm32f3x.cfg"}, {0x432, "stm32f3x.cfg"}, {0x438, "stm32f3x.cfg"}, {0x439, "stm32f3x.cfg"}, {0x446, "stm32f3x.cfg"},
        {0x413, "stm32f4x.cfg"}, {0x419, "stm32f4x.cfg"}, {0x421, "stm32f4x.cfg"}, {0x423, "stm32f4x.cfg"}, {0x431, "stm32f4x.cfg"}, {0x433, "stm32f4x.cfg"}, {0x434, "stm32f4x.cfg"}, {0x441, "stm32f4x.cfg"}, {0x458, "stm32f4x.cfg"},
        {0x449, "stm32f7x.cfg"}, {0x451, "stm32f7x.cfg"}, {0x452, "stm32f7x.cfg"},
        {0x456, "stm32g0x.cfg"}, {0x460, "stm32g0x.cfg"}, {0x466, "stm32g0x.cfg"}, {0x467, "stm32g0x.cfg"},
        {0x468, "stm32g4x.cfg"}, {0x469, "stm32g4x.cfg"}, {0x479, "stm32g4x.cfg"},
        {0x450, "stm32h7x.cfg"}, {0x480, "stm32h7x.cfg"}, {0x483, "stm32h7x.cfg"},
        {0x417, "stm32l0.cfg"}, {0x425, "stm32l0.cfg"}, {0x447, "stm32l0.cfg"}, {0x457, "stm32l0.cfg"},
        {0x416, "stm32l1.cfg"}, {0x427, "stm32l1.cfg"}, {0x429, "stm32l1.cfg"}, {0x436, "stm32l1.cfg"}, {0x437, "stm32l1.cfg"},
        {0x415, "stm32l4x.cfg"}, {0x435, "stm32l4x.cfg"}, {0x461, "stm32l4x.cfg"}, {0x462, "stm32l4x.cfg"}, {0x464, "stm32l4x.cfg"}, {0x470, "stm32l4x.cfg"}, {0x471, "stm32l4x.cfg"},
        {0x472, "stm32l5x.cfg"},
        {0x476, "stm32u5x.cfg"}, {0x481, "stm32u5x.cfg"},
        {0x495, "stm32wbx.cfg"}, {0x496, "stm32wbx.cfg"},
        {0x497, "stm32wlx.cfg"}
    };

    auto targetSTM32 = STM32_CONFIGS.find(d_ID);
    return (targetSTM32 != STM32_CONFIGS.end()) ? targetSTM32->second : nullptr;
}

DetectionResult DetectedSTM32(int telnetPort)
{
    DetectionResult result;
    result.success = false;

    if (!socketInit())
    {
        result.errMsg = "Failed to initialize sockets";
        return result;
    }

    SocketType sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == InvalidSocket)
    {
        result.errMsg = "Failed to create socket: " + getLastError();
        socketCleanup();
        return result;
    }

    #ifdef _WIN32
        DWORD timeout = 2000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    #else
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    #endif

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(static_cast<uint16_t>(telnetPort));
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) == SocketError)
    {
        result.errMsg = "Failed to connect to OpenOCD on port " + std::to_string(telnetPort) + ": " + getLastError();
        closesocket(sock);
        socketCleanup();
        return result;
    }

    printf("DEBUG: Connected to OpenOCD\n");

    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    int r = recv(sock, buffer, sizeof(buffer) - 1, 0);
    printf("DEBUG: Welcome recv returned %d bytes: [%s]\n", r, buffer);
    if (r > 0)
    {
        printf("DEBUG: Welcome raw bytes:");
        for (int i = 0; i < r; ++i) printf(" %02X", (unsigned char)buffer[i]);
        printf("\n");
    }

    for (auto addr : IDCODE_ADDRS)
    {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "mdw 0x%08X", addr);
        printf("DEBUG: Sending: %s\n", cmd);

        if (sendCommand(sock, cmd, buffer, sizeof(buffer)))
        {
            printf("DEBUG: Response: [%s]\n", buffer);
            uint32_t idcode = parseID(buffer);
            printf("DEBUG: Parsed: 0x%08X, devId: 0x%03X\n", idcode, idcode & 0xFFF);

            if (idcode != 0)
            {
                uint16_t devId = idcode & 0xFFF;
                const char* config = getSTM32Config(devId);

                if (config)
                {
                    result.success = true;
                    result.devID = devId;
                    result.configFileName = config;
                    break;
                }
            }
        }
        else
        {
            printf("DEBUG: sendCommand failed\n");
        }
    }

    if (!result.success && result.errMsg.empty()) result.errMsg = "Unknown or unsupported STM32 device";

    closesocket(sock);
    socketCleanup();

    return result;
}