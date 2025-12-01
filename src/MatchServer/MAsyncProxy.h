#pragma once

#include <deque>
#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>
#include <thread>
#include "GlobalTypes.h"
#include "MSync.h"
#include "function_view.h"
#include "IDatabase.h"

enum MASYNC_RESULT {
	MASYNC_RESULT_SUCCEED,
	MASYNC_RESULT_FAILED,
	MASYNC_RESULT_TIMEOUT
};

class MAsyncJob {
protected:
	int				m_nJobID;	// Job Type ID

	u64	m_nPostTime;
	u64 m_nFinishTime;

	MASYNC_RESULT	m_nResult;

public:
	MAsyncJob(int nJobID) {
		m_nJobID = nJobID;
		m_nPostTime = 0;
		m_nFinishTime = 0;
		m_nResult = MASYNC_RESULT_FAILED; // Inicializar resultado
	}
	virtual ~MAsyncJob()	{}

	int GetJobID()							{ return m_nJobID; }
	auto GetPostTime() const				{ return m_nPostTime; }
	void SetPostTime(u64 nTime)				{ m_nPostTime = nTime; }
	auto GetFinishTime() const				{ return m_nFinishTime; }
	void SetFinishTime(u64 nTime)			{ m_nFinishTime = nTime; }

	MASYNC_RESULT GetResult()				{ return m_nResult; }
	void SetResult(MASYNC_RESULT nResult)	{ m_nResult = nResult; }

	virtual void Run(void* pContext) = 0;
};

class MAsyncJobList : private std::deque<MAsyncJob*> {
protected:
	MCriticalSection m_csLock;
public:
	MAsyncJobList() = default;
	~MAsyncJobList() {
		// Limpiar jobs pendientes si los hay
		// Nota: No usar Lock() aquí porque puede causar deadlock si se llama desde otro thread
		// En su lugar, limpiar directamente sin lock (solo si estamos seguros de que no hay otros threads)
		while (!empty()) {
			MAsyncJob* pJob = front();
			pop_front();
			if (pJob) {
				delete pJob;
			}
		}
	}

	// No copiable ni movible
	MAsyncJobList(const MAsyncJobList&) = delete;
	MAsyncJobList& operator=(const MAsyncJobList&) = delete;
	MAsyncJobList(MAsyncJobList&&) = delete;
	MAsyncJobList& operator=(MAsyncJobList&&) = delete;

	void Lock()		{ m_csLock.lock(); }
	void Unlock()	{ m_csLock.unlock(); }

	auto GetBeginItorUnsafe()	{ return begin(); }
	auto GetEndItorUnsafe()		{ return end(); }

	void AddUnsafe(MAsyncJob* pJob) {
		push_back(pJob);
	}
	void RemoveUnsafe(MAsyncJob* pJob, MAsyncJobList::iterator* itorOut) {
		iterator i = find(begin(), end(), pJob);
		if (i != end()) {
			iterator itorTmp = erase(i);
			if (itorOut)
				*itorOut = itorTmp;
		}
	}
	MAsyncJob* GetJobUnsafe() {
		if (begin() == end()) return NULL;
		MAsyncJob* pReturn = *begin();
		pop_front();
		return pReturn;
	}
	int GetCount() { return (int)size(); }
};

#define MAX_THREADPOOL_COUNT 10

class MAsyncProxy final {
protected:
	MSignalEvent EventShutdown;
	MSignalEvent EventFetchJob;

	MAsyncJobList WaitQueue;
	MAsyncJobList ResultQueue;

	MCriticalSection csCrashDump;
	
	std::vector<std::thread> m_Threads; // Track threads para poder esperarlos
	std::vector<IDatabase*> m_Databases; // Track databases para limpiarlas
	MCriticalSection m_csThreads; // Proteger acceso a threads
	bool m_bDestroyed; // Flag para evitar múltiples llamadas a Destroy()

	void OnRun(IDatabase* Database);

public:
	// Constructor por defecto
	MAsyncProxy() : m_bDestroyed(false) {}
	
	// Destructor - limpia recursos
	~MAsyncProxy();
	
	// No copiable ni movible (regla de tres/cinco)
	MAsyncProxy(const MAsyncProxy&) = delete;
	MAsyncProxy& operator=(const MAsyncProxy&) = delete;
	MAsyncProxy(MAsyncProxy&&) = delete;
	MAsyncProxy& operator=(MAsyncProxy&&) = delete;

	bool Create(int ThreadCount);
	bool Create(int ThreadCount, function_view<IDatabase*()> GetDatabase);
	void Destroy();
	
	int GetWaitQueueCount()		{ return WaitQueue.GetCount(); }
	int GetResultQueueCount()	{ return ResultQueue.GetCount(); }

	void PostJob(MAsyncJob* pJob);
	MAsyncJob* GetJobResult() {
		ResultQueue.Lock();
			auto pJob = ResultQueue.GetJobUnsafe();
		ResultQueue.Unlock();
		return pJob;
	}
};
