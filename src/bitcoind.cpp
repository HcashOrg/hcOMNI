// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "chainparams.h"
#include "clientversion.h"
#include "httprpc.h"
#include "httpserver.h"
#include "init.h"
#include "noui.h"
#include "rpc/server.h"
#include "scheduler.h"
#include "util.h"
#include "utilstrencodings.h"

#include "omnicore/utilsui.h"
#include "omnicore/omnicore.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include "rpc/register.h"

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif // WIN32

//#include "createpayload.h"
/* Introduction text for doxygen: */

/*! \mainpage Developer documentation
 *
 * \section intro_sec Introduction
 *
 * This is the developer documentation of the reference client for an experimental new digital currency called Bitcoin (https://www.bitcoin.org/),
 * which enables instant payments to anyone, anywhere in the world. Bitcoin uses peer-to-peer technology to operate
 * with no central authority: managing transactions and issuing money are carried out collectively by the network.
 *
 * The software is a community-driven open source project, released under the MIT license.
 *
 * \section Navigation
 * Use the buttons <code>Namespaces</code>, <code>Classes</code> or <code>Files</code> at the top of the page to start navigating the code.
 */

static bool fDaemon;

#ifdef WIN32

LONG __stdcall ExceptCallBack(EXCEPTION_POINTERS *pExcPointer)
{
	time_t nowtime = time(NULL);
	std::string fileName;
	char buf[21] = {0};
	fileName += _i64toa_s(nowtime, buf, 20, 10);
	fileName += ".dmp";
	boost::filesystem::path filePath = GetDataDir(false)/fileName;
	HANDLE hFile = CreateFileA(filePath.string().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
	loExceptionInfo.ExceptionPointers = pExcPointer;
	loExceptionInfo.ThreadId = GetCurrentThreadId();
	loExceptionInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
	CloseHandle(hFile);
	return EXCEPTION_EXECUTE_HANDLER;
}


#endif // WIN32

//void WaitForShutdown(boost::thread_group* threadGroup)
//{
//    bool fShutdown = ShutdownRequested();
//    // Tell the main threads to shutdown.
//    while (!fShutdown) {
//        MilliSleep(200);
//        fShutdown = ShutdownRequested();
//    }
//    if (threadGroup) {
//        Interrupt(*threadGroup);
//        threadGroup->join_all();
//    }
//}

//////////////////////////////////////////////////////////////////////////////
//
// Start
//
bool AppInitEx(char* netName, char* dataDir)
{
	if (dataDir == NULL)
	{
		return false;
	}
	mapArgs["-datadir"] = dataDir;
	if (!boost::filesystem::is_directory(GetDataDir(false))) {
		return false;
    }
	if( netName ){
		SelectParams(netName);
	}

#ifdef WIN32
	SetUnhandledExceptionFilter(ExceptCallBack);

#endif // WIN32

	RegisterAllCoreRPCCommands(tableRPC);
	SetRPCWarmupFinished();
	mastercore_init_ex();
	return true;
}
//////////////////////////////////////////////////////////////////////////////
//
// Start
//
//bool AppInit(int argc, char* argv[])
//{
//    boost::thread_group threadGroup;
//    CScheduler scheduler;
//
//    bool fRet = false;
//
//    //
//    // Parameters
//    //
//    // If Qt is used, parameters/bitcoin.conf are parsed in qt/bitcoin.cpp's main()
//    ParseParameters(argc, argv);
//
//    // Process help and version before taking care about datadir
//    if (mapArgs.count("-?") || mapArgs.count("-h") || mapArgs.count("-help") || mapArgs.count("-version")) {
//        std::string strUsage = strprintf(_("%s Daemon"), _(PACKAGE_NAME)) + " " + _("version") + " " + FormatFullVersion() + "\n";
//
//        if (mapArgs.count("-version")) {
//            strUsage += FormatParagraph(LicenseInfo());
//        } else {
//            strUsage += "\n" + _("Usage:") + "\n" +
//                        "  omnicored [options]                     " + strprintf(_("Start %s Daemon"), _(PACKAGE_NAME)) + "\n";
//
//            strUsage += "\n" + HelpMessage(HMM_BITCOIND);
//        }
//
//        fprintf(stdout, "%s", strUsage.c_str());
//        return true;
//    }
//
//    try {
//        if (!boost::filesystem::is_directory(GetDataDir(false))) {
//            fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", mapArgs["-datadir"].c_str());
//            return false;
//        }
//        try {
//            ReadConfigFile(mapArgs, mapMultiArgs);
//        } catch (const std::exception& e) {
//            fprintf(stderr, "Error reading configuration file: %s\n", e.what());
//            return false;
//        }
//        // Check for -testnet or -regtest parameter (Params() calls are only valid after this clause)
//        try {
//            SelectParams(ChainNameFromCommandLine());
//        } catch (const std::exception& e) {
//            fprintf(stderr, "Error: %s\n", e.what());
//            return false;
//        }
//
//        // Command-line RPC
//        bool fCommandLine = false;
//        for (int i = 1; i < argc; i++)
//            if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "bitcoin:"))
//                fCommandLine = true;
//
//        if (fCommandLine) {
//            fprintf(stderr, "Error: There is no RPC client functionality in omnicored anymore. Use the omnicore-cli utility instead.\n");
//            exit(EXIT_FAILURE);
//        }
//#ifndef WIN32
//        fDaemon = GetBoolArg("-daemon", false);
//        if (fDaemon) {
//            fprintf(stdout, "Omni Core server starting\n");
//
//            // Daemonize
//            pid_t pid = fork();
//            if (pid < 0) {
//                fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
//                return false;
//            }
//            if (pid > 0) // Parent process, pid is child process id
//            {
//                return true;
//            }
//            // Child process falls through to rest of initialization
//
//            pid_t sid = setsid();
//            if (sid < 0)
//                fprintf(stderr, "Error: setsid() returned %d errno %d\n", sid, errno);
//        }
//#endif
//        SoftSetBoolArg("-server", true);
//
//        // Set this early so that parameter interactions go to console
//        InitLogging();
//        InitParameterInteraction();
//        fRet = AppInit2(threadGroup, scheduler);
//    } catch (const std::exception& e) {
//        PrintExceptionContinue(&e, "AppInit()");
//    } catch (...) {
//        PrintExceptionContinue(NULL, "AppInit()");
//    }
//
//    if (!fRet) {
//        Interrupt(threadGroup);
//        // threadGroup.join_all(); was left out intentionally here, because we didn't re-test all of
//        // the startup-failure cases to make sure they don't result in a hang due to some
//        // thread-blocking-waiting-for-another-thread-during-startup case
//    } else {
//        WaitForShutdown(&threadGroup);
//    }
//    Shutdown();
//
//    return fRet;
//}
//
//
int main_actual(int argc, char* argv[])
{
    SetupEnvironment();

    // Indicate no-UI mode
    //fQtMode = false;

    // Connect bitcoind signal handlers
    //noui_connect();

    //return (AppInit(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE);
}



 
