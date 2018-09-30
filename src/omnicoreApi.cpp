#include "omnicoreApi.h"
#include <vector>

#ifdef _WIN32
#ifdef MYDLL_IMPORTS
#define MYDLLAPI extern "C" __declspec(dllimport)
#else
#define MYDLLAPI extern "C" __declspec(dllexport)
#endif
#else
#define MYDLLAPI extern "C"
#endif

extern void PropertyToJSON(const CMPSPInfo::Entry& sProperty, UniValue& property_obj);


MYDLLAPI const char* JsonCmdReq(char* pcReq)
{
    std::string strReq = std::string(pcReq);
    std::string strReply = HTTPReq_JSONRPC_Simple(strReq);
    static char acTemp[512000];
    if (strReply.size() < sizeof(acTemp)) {
        memset(acTemp, 0, sizeof(acTemp));
        strncpy(acTemp, strReply.c_str(), sizeof(acTemp) - 1);
        return acTemp;
    } else {
        static std::vector<char> vReply;
        vReply.resize(strReply.size() + 1);
        std::copy(strReply.begin(), strReply.end(), vReply.begin());
        return (&vReply[0]);
    }
}

typedef char* (*FunJsonCmdReqOmToHc)(char*);
FunJsonCmdReqOmToHc gFunJsonCmdReqOmToHc;
#ifdef _WIN32
char* JsonCmdReqOmToHc(char* pcReq)  //linux version is defined in omniSubCall_unix.go
{
    if (gFunJsonCmdReqOmToHc == NULL)
        return NULL;
    char* pcRsp = gFunJsonCmdReqOmToHc(pcReq);
    return pcRsp;
}
#endif

MYDLLAPI void SetCallback(unsigned int uiIndx, void* pJsonCmdReqOmToHc)
{ //	now just set one callback function,other may add later

    gFunJsonCmdReqOmToHc = (FunJsonCmdReqOmToHc)pJsonCmdReqOmToHc;

    if (gFunJsonCmdReqOmToHc == NULL) {
        printf("in C gFunJsonCmdReqOmToHc==NULL");
        return;
    }

	//char* pcRsp = JsonCmdReqOmToHc("{\"jsonrpc\" : \"1.0\", \"method\" : \"getinfo\", \"params\" : [], \"id\" : 1}");
    //printf("in C JsonCmdReqOmToHc pcRsp=%s", pcRsp);
}



int parse_cmdline(char* line, char*** argvp)
{
    char** argv = (char**)malloc(sizeof(char*));
    int argc = 0;
    while (*line != '\0') {
        char quote = 0;
        while (strchr("\t ", *line)) /* Skips white spaces */
            line++;
        if (!*line)
            break;
        argv = (char**)realloc(argv, (argc + 2) * sizeof(char*)); /* Starts a new parameter */
        if (*line == '"') {
            quote = '"';
            line++;
        }
        argv[argc++] = line;
    more:
        while (*line && !strchr("\t ", *line))
            line++;

        if (line > argv[argc - 1] && line[-1] == quote) // End of quoted parameter
            line[-1] = 0;
        else if (*line && quote) { // Space within a quote

            line++;
            goto more;
        } else // End of unquoted parameter
            if (*line)
            *line++ = 0;
    }
    argv[argc] = NULL;
    *argvp = argv;
    return argc;
}

extern bool AppInitEx(char* netName);

MYDLLAPI void OmniStart(char* netName)
{
    AppInitEx(netName);
    /*
    printf(" in OmniStart\n");

	char* argv[] = {"-exe", "-regtest", "-txindex", "-server=1", "-addnode=192.168.1.24", "-reindex-chainstate", "-debug=1"};
   //  int argc = parse_cmdline(7, &argv);


    //below copy from main()
    //char* argv[] = {"exeName", "-regtest", "-txindex"};
    SetupEnvironment();
  //  fQtMode = false; // Indicate no-UI mode
    noui_connect();  // Connect bitcoind signal handlers
    AppInit(7, argv);
    //main(3, argv);
	*/
}
