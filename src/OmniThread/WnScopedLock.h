#ifndef _WN_SCOPEDLOCK_H
#define _WN_SCOPEDLOCK_H

#include "WnLock.h"

class CWnScopedLock : public CWnLock
{
public:
  CWnScopedLock(CWnLock& mutex);
  virtual ~CWnScopedLock(void);
public:
  virtual void lock() {return m_pMutex->lock();}
  virtual void unlock() {return m_pMutex->unlock();}
private:
  CWnLock* m_pMutex;
};

#endif //_WN_SCOPEDLOCK_H