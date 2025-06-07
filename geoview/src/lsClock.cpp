#include "lsClock.h"
#include "lsTime.hpp"


lsTime lsClock::getElapsedTime() const
{
	if (isRunning())
		return std::chrono::duration_cast<std::chrono::microseconds>(ClockImpl::now() - m_refPoint);
	return std::chrono::duration_cast<std::chrono::microseconds>(m_stopPoint - m_refPoint);
}

bool lsClock::isRunning() const
{
	return m_stopPoint == ClockImpl::time_point();
}

void lsClock::start()
{
	if (!isRunning())
	{
		m_refPoint += ClockImpl::now() - m_stopPoint;
		m_stopPoint = {};
	}
}

void lsClock::stop()
{
	if (isRunning())
		m_stopPoint = ClockImpl::now();
}

lsTime lsClock::restart()
{
	const lsTime elapsed = getElapsedTime();
	m_refPoint = ClockImpl::now();
	m_stopPoint = {};
	return elapsed;
}

lsTime lsClock::reset()
{
	const lsTime elapsed = getElapsedTime();
	m_refPoint = ClockImpl::now();
	m_stopPoint = m_refPoint;
	return elapsed;
}
