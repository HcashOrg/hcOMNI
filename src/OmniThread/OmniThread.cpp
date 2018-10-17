
#include "OmniThread.h"
#include "omnicore/persistence.h"
#include "omnicore/log.h"

CDataNotify::CDataNotify() :
	type(0), data("")
{

}
CDataNotify::CDataNotify(int tp, const std::string& dt):
	type(tp), data(dt)
{
}

CDllDataHandler::CDllDataHandler(void)
{
}

CDllDataHandler::~CDllDataHandler()
{
}

void CDllDataHandler::SetNetName(const char* name)
{

}

void CDllDataHandler::DataFunc(CDataNotify& data)
{
	switch (data.type)
	{
	case CDataNotify::INIT_THREAD:
		InitThread();
	break;
	case CDataNotify::ON_BLOCK_CONNECTED_NOTIFY:
	{
		OnBlockConnected(data);
	} 
	break;
	}
}

void CDllDataHandler::InitThread()
{
	
}

void CDllDataHandler::OnBlockConnected(CDataNotify& data)
{
	//PrintToConsole("CDllDataHandler::OnBlockConnected hash = %s\n", data.data);
	write_files(data.data);
}

  