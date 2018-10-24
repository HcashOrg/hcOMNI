#ifndef _DATAHANDLER_H
#define _DATAHANDLER_H

#include "WnQueue.h"
#include "WnEvent.h"
#include <boost/thread.hpp>

// a pure virtual function
template <class T> class CDataHandler
{
public:
	CDataHandler(void) {Start();}
	virtual ~CDataHandler() {}

public:
	void NotifyOne() {m_event.notify_one();}

	void NotifyAll() {m_event.notify_all();}

	void Put(const T& t)
	{
		m_record_set.push(t);

		m_event.notify_one();
	}

	int BufferSize() { return m_record_set.size(); }

	void Stop()
	{
		m_thread.interrupt();
		m_thread.join();
	}

protected:
	virtual void DataThread()
	{
		try  
		{  
			while(true)
			{
				boost::this_thread::interruption_point();
				m_event.wait();
				while(!m_record_set.empty())
				{
					T t;
					if(m_record_set.get(t))
					{
						DataFunc(t);
					}
				}
			}
		}  
		catch(...)  
		{
		}
	}

	virtual void DataFunc(T& t) {}


protected:
	void Start()
	{
		m_thread = boost::thread(&CDataHandler::DataThread, this);  
	}

protected:
	// data
	CWnQueue<T> m_record_set;

	// event
	CWnEvent m_event;	
	//thread
	boost::thread m_thread;  

};

#endif //_DATAHANDLER_H