#ifndef _WN_LOCK_H
#define _WN_LOCK_H

#include <boost/thread.hpp>

class CWnLock
{
public:
	CWnLock(void) {}
	~CWnLock(void) {}
public:
	virtual void lock() {m_lock.lock();}
	virtual void unlock() {m_lock.unlock();}
protected:
  boost::mutex m_lock;
};

#endif //_WN_LOCK_H