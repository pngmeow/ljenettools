#define LJE_SDK_IMPLEMENTATION
#include "lje_sdk.h"
#include "MinHook.h"
#include "log.h"
#include "sigscan.hpp"
#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <sigs.h>


// globals
bool* fnBSendPacket  = nullptr;
int* iChokedCommands = nullptr;

using HostAccumulateTimeFn = void(__cdecl*)(float);
HostAccumulateTimeFn oHostAccumulateTime = nullptr;

// options
float g_Timescale = 1.0f;
bool g_EnableTimescale = false;

// signatures
uintptr_t engine_base = 0;
uintptr_t bSendPacket = 0;
uintptr_t chokedCommands = 0;
uintptr_t hostAccumulateTime = 0;

static LjeApi* g_ljeApi = NULL;

void hkHostAccumulateTime(float dt)
{
    if (g_EnableTimescale)
    {
        dt *= g_Timescale;
    }

    oHostAccumulateTime(dt);
}

bool InstallHostAccumulateTimeHook()
{
    logger::debug("Attempt to hook host_accumulatetime");

    if (!hostAccumulateTime)
    {
        logger::error("hostAccumulateTime is invalid!");
        return false;
    }

    // Resolve CALL target
    int32_t rel = *(int32_t*)(hostAccumulateTime + 1);
    uintptr_t targetFunc = hostAccumulateTime + 5 + rel;
    logger::debug("hostAccumulateTime targetFunc = %p", targetFunc);

    if (MH_Initialize() != MH_OK)
        return false;

    logger::debug("Hooking hostAccumulateTime...");
    if (MH_CreateHook((void*)targetFunc, hkHostAccumulateTime, (void**)&oHostAccumulateTime) != MH_OK)
        return false;

    if (MH_EnableHook((void*)targetFunc) != MH_OK)
        return false;

    return true;
}

LJE_MODULE_INIT()
{
    engine_base = battlehook::helper::get_module_base("engine.dll");
    uintptr_t CL_Move = battlehook::helper::find_pattern("engine.dll", SIG_CLMOVE);
    bSendPacket = CL_Move + 0x62; // offset to 'mov dil, 1' byte

	chokedCommands = battlehook::helper::find_pattern("engine.dll", SIG_CHOKEDCOMMANDS);
	hostAccumulateTime = battlehook::helper::find_pattern("engine.dll", SIG_HOSTACCUMULATETIME);

    int32_t rel = *reinterpret_cast<int32_t*>(chokedCommands + 3);
    iChokedCommands = reinterpret_cast<int*>(chokedCommands + 7 + rel); //another offset
    fnBSendPacket = reinterpret_cast<bool*>(bSendPacket);


    logger::info(R"(
           -=-
        (\  _  /)
        ( \( )/ )       ljenettools
        (       )       by pngmeow
         `>   <'
         /     \  
         `-._.-'

    MAKE SURE YOU DOWNLOAD THIS MODULE OFF https://github.com/pngmeow/ljenettools.
    ANY OTHER SOURCE IS LIKELY TO BE MALICIOUS AND NOT WORK AS INTENDED.
    )");



    logger::debug("engine.dll = %p", engine_base);

    if (!engine_base)
        return LJE_RESULT_ERROR;

    logger::debug("bSendPacket = %p", bSendPacket);
    logger::debug("chokedCommands = %p", chokedCommands);
    logger::debug("host_accumulate_time = %p", hostAccumulateTime);

    if (!bSendPacket || !chokedCommands || !hostAccumulateTime)
    {
        logger::error("Failed to resolve patterns!");
        return LJE_RESULT_ERROR;
    }

    logger::info("installing hook");
    if (!InstallHostAccumulateTimeHook())
    {
        logger::error("Something went wrong (hook failed)");
        return LJE_RESULT_ERROR;
    }

    logger::info("hook ok :3");

    if (fnBSendPacket)
    {
        DWORD old;
        VirtualProtect(fnBSendPacket, 1, PAGE_EXECUTE_READWRITE, &old);
    }


    g_ljeApi = api;

    if (api->version != LJE_SDK_VERSION)
    {
        logger::error("Something went wrong (LJE SDK MISMATCH)");
        return LJE_RESULT_INCOMPATIBLE_SDK_VERSION;
    }

    logger::info("Done!");
    return LJE_RESULT_OK;
}

static int lua_enable_timescale(lua_State* L)
{
    LjeLuaApi* lua = g_ljeApi->lua;

    if (lua->gettop(L) >= 1)
        g_EnableTimescale = lua->toboolean(L, 1);

    lua->pushboolean(L, g_EnableTimescale);
    return 1;
}

static int lua_set_timescale(lua_State* L)
{
    LjeLuaApi* lua = g_ljeApi->lua;

    if (lua->gettop(L) >= 1)
        g_Timescale = (float)lua->tonumber(L, 1);

    lua->pushnumber(L, g_Timescale);
    return 1;
}

static int lua_send_packet(lua_State* L) {
    if (!fnBSendPacket)
        return 0;

    LjeLuaApi* lua = g_ljeApi->lua;
    int nargs = lua->gettop(L);

    if (nargs >= 1) {
        bool value = lua->toboolean(L, 1);
        *fnBSendPacket = value;
    }

    lua->pushboolean(L, *fnBSendPacket);
    return 1;
}

static int lua_choked_packets(lua_State* L) {
    if (!iChokedCommands)
        return 0;

    LjeLuaApi* lua = g_ljeApi->lua;
    lua->pushnumber(L, *iChokedCommands);
    return 1;
}

LJE_MODULE_PREINIT()
{
    

    LjeLuaApi* lua = g_ljeApi->lua;
    lua->pushljeenv(L); 

    lua->pushcclosure(L, lua_enable_timescale, 0);
    lua->setfield(L, -2, "enable_timescale");

    lua->pushcclosure(L, lua_set_timescale, 0);
    lua->setfield(L, -2, "set_timescale");

    lua->pushcclosure(L, lua_send_packet, 0);
    lua->setfield(L, -2, "send_packet");

    lua->pushcclosure(L, lua_choked_packets, 0);
    lua->setfield(L, -2, "choked_commands");

    lua->pop(L, 1); 

    return LJE_RESULT_OK;
}

int LJE_MODULE_UNLOAD()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    return LJE_RESULT_OK;
}