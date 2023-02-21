#include "simplelink.h"
#include "sl_common.h"
#include <stdio.h>
#include <string.h>

/**
 * Values for below macros shall be modified per the access-point's (AP) properties
 * SimpleLink device will connect to following AP when the application is executed.
 */
#define SSID_NAME       ""       /* Access point name to connect to. */
#define SEC_TYPE        SL_SEC_TYPE_WPA_WPA2     /* Security type of the Access piont */
#define PASSKEY         ""   /* Password in case of secure AP */
#define PASSKEY_LEN     pal_Strlen(PASSKEY)      /* Password length in case of secure AP */

/* Http request variables. */
#define SL_STOP_TIMEOUT        0xFF

#define MOCK_SERVER  "cctest.free.beeceptor.com"

#define PREFIX_BUFFER   "GET /my/api/path"
#define POST_BUFFER     " HTTP/1.1\r\nHost:cctest.free.beeceptor.com\r\nAccept: */"
#define POST_BUFFER2    "*\r\n\r\n"

#define SMALL_BUF           32
#define MAX_SEND_BUF_SIZE   512
#define MAX_SEND_RCV_SIZE   300

/* Application specific status/error codes. */
typedef enum
{
    DEVICE_NOT_IN_STATION_MODE = -0x7D0, /* Choosing this number to avoid overlap with host-driver's error codes */
    HTTP_SEND_ERROR = DEVICE_NOT_IN_STATION_MODE - 1,
    HTTP_RECV_ERROR = HTTP_SEND_ERROR - 1,
    HTTP_INVALID_RESPONSE = HTTP_RECV_ERROR - 1,
    STATUS_CODE_MAX = -0xBB8
} e_AppStatusCodes;

/* GLOBAL VARIABLES. */

_u32 g_Status = 0;

/* Request buffer to send/receive data from server. */
struct
{
    _u8 Recvbuff[MAX_SEND_RCV_SIZE];
    _u8 SendBuff[MAX_SEND_BUF_SIZE];

    _u8 HostName[SMALL_BUF];

    _u32 DestinationIP;
    _i16 SockID;
} g_AppData;

/* Static functions definition. */
static _i32 establishConnectionWithAP();
static _i32 disconnectFromAP();
static _i32 configureSimpleLinkToDefaultState();
static _i32 initializeAppVariables();
static _i32 getHostIP();
static _i32 createConnection();
static _i32 getResponse();
static _i32 getData();

/* ASYNCHRONOUS EVENT HANDLERS. */

/* This function handles WLAN events. */
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    switch (pWlanEvent->Event)
    {
    case SL_WLAN_CONNECT_EVENT:
    {
        SET_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
    }
        break;

    case SL_WLAN_DISCONNECT_EVENT:
    {
        slWlanConnectAsyncResponse_t *pEventData = NULL;

        CLR_STATUS_BIT(g_Status, STATUS_BIT_CONNECTION);
        CLR_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);

        pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

        /* If the user has initiated 'Disconnect' request, 'reason_code' is SL_USER_INITIATED_DISCONNECTION */
        if (SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
        {
            //Device disconnected from the AP on application's request.
        }
        else
        {
            //Device disconnected from the AP on an ERROR.
        }
    }
        break;
    }
}

/* This function handles events for IP address acquisition via DHCP indication. */
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    switch (pNetAppEvent->Event)
    {
    case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
    {
        SET_STATUS_BIT(g_Status, STATUS_BIT_IP_ACQUIRED);
    }
        break;
    }
}

/**
 * This function handles callback for the HTTP server events.
 * This application doesn't work with HTTP server so we don't define it.
 */
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
{

}

/**
 * This function handles general error events indication.
 * Most of the general errors are not FATAL are are to be handled
 * appropriately by the application.
 * */
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{

}

/* This function handles socket events indication. */
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    switch (pSock->Event)
    {
    case SL_SOCKET_TX_FAILED_EVENT:
    {
        switch (pSock->EventData.status)
        {
        case SL_ECLOSE:
            //Close socket operation failed to transmit all queued packets.
            break;
        }
    }
        break;
    }
}

/* MAIN FUNCTION. */
int main(int argc, char **argv)
{
    _i32 retVal = -1;

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    /* Stop WDT and initialize the system-clock of the MCU. */
    stopWDT();
    initClk();

    retVal = configureSimpleLinkToDefaultState();
    if (retVal < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == retVal)
            //Failed to configure the device in its default state.
            LOOP_FOREVER();
    }

    retVal = sl_Start(0, 0, 0);
    if ((retVal < 0) || (ROLE_STA != retVal))
    {
        //" Failed to start the device.
        LOOP_FOREVER();
    }

    /* Connecting to WLAN AP. */
    retVal = establishConnectionWithAP();
    if (retVal < 0)
    {
        //Failed to establish connection with an AP.
        LOOP_FOREVER();
    }

    /* Gets the response from the http receive buffer. */
    retVal = getResponse();
    if (retVal < 0)
    {
        //Failed to get weather information.
        LOOP_FOREVER();
    }

    /* Disconnects from the AP. */
    retVal = disconnectFromAP();
    if (retVal < 0)
    {
        //Failed to disconnect from the AP.
        LOOP_FOREVER();
    }

    return 0;
}

/* Gets the Server IP address, zero for success and -1 for error. */
static _i32 getHostIP()
{
    _i32 status = 0;

    status = sl_NetAppDnsGetHostByName((_i8*) g_AppData.HostName,
                                       pal_Strlen(g_AppData.HostName),
                                       &g_AppData.DestinationIP, SL_AF_INET);
    ASSERT_ON_ERROR(status);

    return SUCCESS;
}
/* Get information from the server. */
static _i32 getResponse()
{
    _i32 retVal = -1;

    pal_Strcpy((char* )g_AppData.HostName, MOCK_SERVER);

    retVal = getHostIP();
    if (retVal < 0)
    {
        //Unable to reach host.
        ASSERT_ON_ERROR(retVal);
    }

    g_AppData.SockID = createConnection();
    ASSERT_ON_ERROR(g_AppData.SockID);

    retVal = getData();
    ASSERT_ON_ERROR(retVal);

    retVal = sl_Close(g_AppData.SockID);
    ASSERT_ON_ERROR(retVal);

    return 0;
}
/* Creates UDP socket to communicate with server. */
static _i32 createConnection()
{
    SlSockAddrIn_t Addr;

    _i16 sd = 0;
    _i16 AddrSize = 0;
    _i32 ret_val = 0;

    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(80);

    /* Change the DestinationIP endianity, to big endian */
    Addr.sin_addr.s_addr = sl_Htonl(g_AppData.DestinationIP);

    AddrSize = sizeof(SlSockAddrIn_t);

    sd = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, 0);
    if (sd < 0)
    {
        //Error creating socket.
        ASSERT_ON_ERROR(sd);
    }

    ret_val = sl_Connect(sd, (SlSockAddr_t*) &Addr, AddrSize);
    if (ret_val < 0)
    {
        //Error connecting to server.
        ASSERT_ON_ERROR(ret_val);
    }

    return sd;
}
/* This function Obtains the required data from the server. */
static _i32 getData()
{
    _u8 *p_startPtr = NULL;
    _u8 *p_bufLocation = NULL;
    _u8 *p_endPtr = NULL;
    _i32 retVal = -1;

    pal_Memset(g_AppData.Recvbuff, 0, sizeof(g_AppData.Recvbuff));

    /* Puts together the HTTP GET string. */
    p_bufLocation = g_AppData.SendBuff;
    pal_Strcpy(p_bufLocation, PREFIX_BUFFER);

    p_bufLocation += pal_Strlen(PREFIX_BUFFER);
    pal_Strcpy(p_bufLocation, POST_BUFFER);

    p_bufLocation += pal_Strlen(POST_BUFFER);
    pal_Strcpy(p_bufLocation, POST_BUFFER2);

    /* Send the HTTP GET string to the open TCP/IP socket. */
    retVal = sl_Send(g_AppData.SockID, g_AppData.SendBuff,
                     pal_Strlen(g_AppData.SendBuff), 0);
    if (retVal != pal_Strlen(g_AppData.SendBuff))
        ASSERT_ON_ERROR(HTTP_SEND_ERROR);

    /* Receive response. */
    retVal = sl_Recv(g_AppData.SockID, &g_AppData.Recvbuff[0],
    MAX_SEND_RCV_SIZE,
                     0);
    if (retVal <= 0)
    {
        ASSERT_ON_ERROR(HTTP_RECV_ERROR);
    }

    g_AppData.Recvbuff[pal_Strlen(g_AppData.Recvbuff)] = '\0';

    return SUCCESS;
}

/** This function configure the SimpleLink device in its default state. It:
 * - Sets the mode to STATION;
 * - Configures connection policy to Auto and AutoSmartConfig;
 * - Deletes all the stored profiles;
 * - Enables DHCP;
 * - Disables Scan policy;
 * - Sets Tx power to maximum;
 * - Sets power policy to normal;
 * - Unregisters mDNS services;
 * - Remove all filters.
 */
static _i32 configureSimpleLinkToDefaultState()
{
    SlVersionFull ver = { 0 };
    _WlanRxFilterOperationCommandBuff_t RxFilterIdMask = { 0 };

    _u8 val = 1;
    _u8 configOpt = 0;
    _u8 configLen = 0;
    _u8 power = 0;

    _i32 retVal = -1;
    _i32 mode = -1;

    mode = sl_Start(0, 0, 0);
    ASSERT_ON_ERROR(mode);

    /* If the device is not in station-mode, try configuring it in station-mode. */
    if (ROLE_STA != mode)
    {
        if (ROLE_AP == mode)
        {
            /* If the device is in AP mode, we need to wait for this event before doing anything. */
            while (!IS_IP_ACQUIRED(g_Status))
            {
                _SlNonOsMainLoopTask();
            }
        }

        /* Switch to STA role and restart. */
        retVal = sl_WlanSetMode(ROLE_STA);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Stop(SL_STOP_TIMEOUT);
        ASSERT_ON_ERROR(retVal);

        retVal = sl_Start(0, 0, 0);
        ASSERT_ON_ERROR(retVal);

        /* Check if the device is in station again. */
        if (ROLE_STA != retVal)
        {
            /* We don't want to proceed if the device is not coming up in station-mode. */
            ASSERT_ON_ERROR(DEVICE_NOT_IN_STATION_MODE);
        }
    }

    /* Get the device's version-information. */
    configOpt = SL_DEVICE_GENERAL_VERSION;
    configLen = sizeof(ver);
    retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION, &configOpt, &configLen,
                       (_u8*) (&ver));
    ASSERT_ON_ERROR(retVal);

    /* Set connection policy to Auto + SmartConfig (Device's default connection policy). */
    retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION,
                              SL_CONNECTION_POLICY(1, 0, 0, 0, 1), NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove all profiles. */
    retVal = sl_WlanProfileDel(0xFF);
    ASSERT_ON_ERROR(retVal);

    /* Device in station-mode. Disconnect previous connection if any. */
    retVal = sl_WlanDisconnect();
    if (0 == retVal)
    {
        /* Wait until the device is connected. */
        while (IS_CONNECTED(g_Status))
        {
            _SlNonOsMainLoopTask();
        }
    }

    /* Enable DHCP client. */
    retVal = sl_NetCfgSet(SL_IPV4_STA_P2P_CL_DHCP_ENABLE, 1, 1, &val);
    ASSERT_ON_ERROR(retVal);

    /* Disable scan. */
    configOpt = SL_SCAN_POLICY(0);
    retVal = sl_WlanPolicySet(SL_POLICY_SCAN, configOpt, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Set Tx power level for station mode to maxilum power. */
    power = 0;
    retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID,
    WLAN_GENERAL_PARAM_OPT_STA_TX_POWER,
                        1, (_u8*) &power);
    ASSERT_ON_ERROR(retVal);

    /* Set PM policy to normal. */
    retVal = sl_WlanPolicySet(SL_POLICY_PM, SL_NORMAL_POLICY, NULL, 0);
    ASSERT_ON_ERROR(retVal);

    /* Unregister mDNS services. */
    retVal = sl_NetAppMDNSUnRegisterService(0, 0);
    ASSERT_ON_ERROR(retVal);

    /* Remove  all 64 filters (8*8). */
    pal_Memset(RxFilterIdMask.FilterIdMask, 0xFF, 8);
    retVal = sl_WlanRxFilterSet(SL_REMOVE_RX_FILTER, (_u8*) &RxFilterIdMask,
                                sizeof(_WlanRxFilterOperationCommandBuff_t));
    ASSERT_ON_ERROR(retVal);

    retVal = sl_Stop(SL_STOP_TIMEOUT);
    ASSERT_ON_ERROR(retVal);

    retVal = initializeAppVariables();
    ASSERT_ON_ERROR(retVal);

    return retVal;
}

/**
 * Connecting to a WLAN Access point.
 * This function connects to the required AP (SSID_NAME).
 * The function will return once we are connected and have acquired IP address.
 * The function waits until the WLAN connection happens or if it acquires an IP address.
 */
static _i32 establishConnectionWithAP()
{
    SlSecParams_t secParams = { 0 };
    _i32 retVal = 0;

    /* Retrieve password to log in. */
    secParams.Key = PASSKEY;
    secParams.KeyLen = PASSKEY_LEN;
    secParams.Type = SEC_TYPE;

    /* Connects to the WLAN. */
    retVal = sl_WlanConnect(SSID_NAME, pal_Strlen(SSID_NAME), 0, &secParams, 0);
    ASSERT_ON_ERROR(retVal);

    while ((!IS_CONNECTED(g_Status)) || (!IS_IP_ACQUIRED(g_Status)))
    {
        _SlNonOsMainLoopTask();
    }

    return SUCCESS;
}

/**
 * Disconnecting from a WLAN Access point
 * If the WLAN disconnection fails, the funtion loop.
 */
static _i32 disconnectFromAP()
{
    _i32 retVal = -1;

    retVal = sl_WlanDisconnect();
    if (0 == retVal)
    {
        /* Wait */
        while (IS_CONNECTED(g_Status))
        {
            _SlNonOsMainLoopTask();
        }
    }

    return SUCCESS;
}

/* This function initializes the application variables. */
static _i32 initializeAppVariables()
{
    g_Status = 0;
    pal_Memset(&g_AppData, 0, sizeof(g_AppData));

    return SUCCESS;
}
