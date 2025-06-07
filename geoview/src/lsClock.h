#pragma once

#include <chrono>

using ClockImpl = std::conditional_t<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>;

class lsTime;

class lsClock
{
public:
	lsTime getElapsedTime() const;

	bool isRunning() const;

	void start();

	void stop();

	lsTime restart();

	lsTime reset();

private:
	ClockImpl::time_point m_refPoint{ClockImpl::now()};
	ClockImpl::time_point m_stopPoint;
};
