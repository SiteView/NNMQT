/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 22:56:41 +0100 (Do, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of mutexes.
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_THREAD_H
#define OGDF_THREAD_H

#include <ogdf/basic/basic.h>

#include <stdint.h>

#ifdef OGDF_SYSTEM_WINDOWS
#include <process.h>
#else
#include <pthread.h>
#endif

namespace ogdf {

#ifdef OGDF_SYSTEM_WINDOWS

class Thread
{
public:
	enum State { tsRunning, tsSuspended };
	enum Priority {
		tpIdle     = -15,
		tpLowest   = -2,
		tpLow      = -1,
		tpNormal   = 0,
		tpHigh     = 1,
		tpHighest  = 2,
		tpCritical = 15
	};

	Thread() : m_handle(0), m_id(0) { }
	~Thread() { CloseHandle(m_handle); }

	bool started() const { return m_id != 0; }

	void priority(Priority p) { SetThreadPriority(m_handle,p); }

	Priority priority() const { return (Priority)GetThreadPriority(m_handle); }

	__uint64 cpuAffinity(__uint64 mask) {
		return SetThreadAffinityMask(m_handle, (DWORD_PTR)mask);
	}

	void start(State state = tsRunning) {
		if(m_handle)
			CloseHandle(m_handle);

		m_handle = (HANDLE) _beginthreadex(0, 0, threadProc, this,
			(state == tsSuspended) ? CREATE_SUSPENDED : 0, &m_id);
	}

	uintptr_t threadID() const { return (uintptr_t)m_id; }

	void start(Priority p, State state = tsRunning) {
		OGDF_ASSERT(m_handle == 0);
		m_handle = (HANDLE) _beginthreadex(0, 0, threadProc, this, CREATE_SUSPENDED, &m_id);
		SetThreadPriority(m_handle,p);
		if(state == tsRunning)
			ResumeThread(m_handle);
	}

	int suspend() {
		return SuspendThread(m_handle);
	}

	int resume() {
		return ResumeThread(m_handle);
	}

	void join() {
		WaitForSingleObject(m_handle,INFINITE);
	}

	bool join(unsigned long milliseconds) {
		return (WaitForSingleObject(m_handle,milliseconds) == WAIT_OBJECT_0);
	}

protected:
	virtual void doWork() = 0;

private:
	static unsigned int __stdcall threadProc(void *pParam) {
		Thread *pThread = static_cast<Thread*>(pParam);
		OGDF_ALLOCATOR::initThread();
		pThread->doWork();
		OGDF_ALLOCATOR::flushPool();
		pThread->m_id = 0;
		_endthreadex(0);
		return 0;
	}

	HANDLE m_handle;
	unsigned int m_id;
};


#else

class Thread
{
public:
	enum State { tsRunning, tsSuspended };
	enum Priority {
		tpIdle     = -15,
		tpLowest   = -2,
		tpLow      = -1,
		tpNormal   = 0,
		tpHigh     = 1,
		tpHighest  = 2,
		tpCritical = 15
	};

	Thread() : m_pt(0) { }

	bool started() const { return m_pt != 0; }

	//void priority(Priority p) { SetThreadPriority(m_handle,p); }

	//Priority priority() const { return (Priority)GetThreadPriority(m_handle); }

	//__uint64 cpuAffinity(__uint64 mask) {
	//	return SetThreadAffinityMask(m_handle, (DWORD_PTR)mask);
	//}

	void start(State state = tsRunning) {
		OGDF_ASSERT(m_pt == 0);
		pthread_create(&m_pt, NULL, threadProc, this);
	}

	uintptr_t threadID() const {
		return (uintptr_t)m_pt;
	}

	//void start(Priority p, State state = tsRunning) {
	//	OGDF_ASSERT(m_handle == 0);
	//	m_handle = (HANDLE) _beginthreadex(0, 0, threadProc, this, CREATE_SUSPENDED, 0);
	//	SetThreadPriority(m_handle,p);
	//	if(state == tsRunning)
	//		ResumeThread(m_handle);
	//}

	//int suspend() {
	//	return SuspendThread(m_handle);
	//}

	//int resume() {
	//	return ResumeThread(m_handle);
	//}

	void join() {
		if(m_pt != 0)
			pthread_join(m_pt,NULL);
	}

	//bool join(unsigned long milliseconds) {
	//	return (WaitForSingleObject(m_handle,milliseconds) == WAIT_OBJECT_0);
	//}

protected:
	virtual void doWork() = 0;

private:
	static void *threadProc(void *pParam) {
		Thread *pThread = static_cast<Thread*>(pParam);
		OGDF_ALLOCATOR::initThread();
		pThread->doWork();
		OGDF_ALLOCATOR::flushPool();
		pThread->m_pt = 0;
		return 0;
	}

	pthread_t m_pt;
};

#endif

} // end namespace ogdf


#endif
