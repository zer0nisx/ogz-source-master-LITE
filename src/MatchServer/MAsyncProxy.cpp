#include "stdafx.h"
#include "MAsyncProxy.h"
#include "MMatchConfig.h"
#include "MMatchServer.h"
#include "MCrashDump.h"
#include "MFile.h"

MAsyncProxy::~MAsyncProxy()
{
	// Asegurar que Destroy() fue llamado antes del destructor
	Destroy();
}

bool MAsyncProxy::Create(int ThreadCount)
{
	return Create(ThreadCount, MakeDatabaseFromConfig);
}

bool MAsyncProxy::Create(int ThreadCount, function_view<IDatabase*()> GetDatabase)
{
	// Limpiar threads anteriores si existen
	Destroy();

	ThreadCount = (ThreadCount < MAX_THREADPOOL_COUNT) ? ThreadCount : MAX_THREADPOOL_COUNT;
	m_bDestroyed = false;

	m_csThreads.lock();
	m_Threads.reserve(ThreadCount);
	m_Databases.reserve(ThreadCount);
	m_csThreads.unlock();

	for (int i = 0; i < ThreadCount; i++)
	{
		IDatabase* pDatabase = GetDatabase();
		if (!pDatabase) {
			// Si falla crear database, limpiar y retornar false
			Destroy();
			return false;
		}

		m_csThreads.lock();
		m_Databases.push_back(pDatabase);
		m_Threads.emplace_back([this, pDatabase] {
			OnRun(pDatabase);
		});
		m_csThreads.unlock();
	}

	return true;
}

void MAsyncProxy::Destroy()
{
	// Evitar múltiples llamadas
	if (m_bDestroyed) {
		return;
	}
	m_bDestroyed = true;

	// Señalar shutdown a todos los threads
	EventShutdown.SetEvent();

	// Esperar a que todos los threads terminen
	m_csThreads.lock();
	for (auto& thread : m_Threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	m_Threads.clear();

	// Limpiar databases
	// Nota: IDatabase no tiene destructor virtual, pero las clases derivadas
	// (SQLiteDatabase, MSSQLDatabase) son 'final' y tienen destructores.
	// Hacer delete aquí es necesario para evitar memory leaks ya que cada
	// thread crea su propia instancia de database con 'new'.
	for (IDatabase* pDatabase : m_Databases) {
		if (pDatabase) {
			delete pDatabase;
		}
	}
	m_Databases.clear();
	m_csThreads.unlock();

	// Limpiar jobs pendientes en WaitQueue
	WaitQueue.Lock();
	while (MAsyncJob* pJob = WaitQueue.GetJobUnsafe()) {
		delete pJob;
	}
	WaitQueue.Unlock();

	// Limpiar jobs pendientes en ResultQueue
	ResultQueue.Lock();
	while (MAsyncJob* pJob = ResultQueue.GetJobUnsafe()) {
		delete pJob;
	}
	ResultQueue.Unlock();
}

void MAsyncProxy::PostJob(MAsyncJob* pJob)
{
	WaitQueue.Lock();
		pJob->SetPostTime(GetGlobalTimeMS());
		WaitQueue.AddUnsafe(pJob);
	WaitQueue.Unlock();

	EventFetchJob.SetEvent();
}

void MAsyncProxy::OnRun(IDatabase* Database)
{
	MSignalEvent* EventArray[]{
		&EventShutdown,
		&EventFetchJob,
	};

	bool bShutdown = false;

	while (!bShutdown)
	{
		const auto Timeout = 1000; // Milliseconds
		const auto WaitResult = WaitForMultipleEvents(EventArray, Timeout);

		if (WaitResult == MSync::WaitTimeout) {
			if (WaitQueue.GetCount() > 0) {
				EventFetchJob.SetEvent();
			}
			continue;
		}

		switch(WaitResult)
		{
		case 0: // Shutdown
			bShutdown = true;
			break;
		case 1:	// Fetch Job
			{
				WaitQueue.Lock();
					MAsyncJob* pJob = WaitQueue.GetJobUnsafe();
				WaitQueue.Unlock();

				if (pJob) {
					pJob->Run(Database);
					pJob->SetFinishTime(GetGlobalTimeMS());

					ResultQueue.Lock();
						ResultQueue.AddUnsafe(pJob);
					ResultQueue.Unlock();
				}

				if (WaitQueue.GetCount() > 0) {
					EventFetchJob.SetEvent();
				}
#ifndef WIN32
				else {
					EventFetchJob.ResetEvent();
				}
#endif
			}
			break;
		};
	};
}
