#pragma once

#include <chrono>
#include <cstdint>

class lsTime
{
public:
	constexpr lsTime() = default;

	template<typename Rep, typename Period>
	constexpr lsTime(const std::chrono::duration<Rep, Period>& duration);

	// 10^0 s
	constexpr float asSeconds() const;

	// 10^-3 s (ms)
	constexpr std::int32_t asMilliseconds() const;

	// 10^-6 s (us)
	constexpr std::int64_t asMicroseconds() const;

	constexpr std::chrono::microseconds toDuration() const;

	template<typename Rep, typename Period>
	constexpr operator std::chrono::duration<Rep, Period>() const;

	static const lsTime Zero;

private:
	std::chrono::microseconds m_microseconds{};
};

#include "lsTime.inl"
