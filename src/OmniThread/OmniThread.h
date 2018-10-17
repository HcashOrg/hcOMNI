#ifndef _OMNI_THREAD_H
#define _OMNI_THREAD_H

#include "DataHandler.h"

class CDataNotify {
public:
	//constructor
	CDataNotify();
	CDataNotify(int tp, const std::string& dt, int h);

	enum { INIT_THREAD = 3, ON_BLOCK_CONNECTED_NOTIFY};
	int type;
	std::string data;
	int height;
};

class CDllDataHandler : public CDataHandler<CDataNotify>
{
public:
	//constructor
	CDllDataHandler(void);

	//destructor
	~CDllDataHandler();
	void SetNetName(const char* name);
protected:
	virtual void DataFunc(CDataNotify& data);
	void InitThread();
private:
	void OnBlockConnected(CDataNotify& data);
private:
	std::string _net_name;
};

#endif //_OMNI_THREAD_H