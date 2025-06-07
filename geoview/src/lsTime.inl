#include "lsTime.hpp"

template<typename Rep, typename Period>
constexpr lsTime::lsTime(const std::chrono::duration<Rep, Period>& duration)
	: m_microseconds(duration)
{
}

constexpr float lsTime::asSeconds() const
{
	return std::chrono::duration<float>(m_microseconds).count();
}

constexpr std::int32_t lsTime::asMilliseconds() const
{
	return std::chrono::duration_cast<std::chrono::duration<std::int32_t, std::milli>>(m_microseconds).count();
}

constexpr std::int64_t lsTime::asMicroseconds() const
{
	return m_microseconds.count();
}

constexpr std::chrono::microseconds lsTime::toDuration() const
{
	return m_microseconds;
}

template<typename Rep, typename Period>
constexpr lsTime::operator std::chrono::duration<Rep, Period>() const
{
	return m_microseconds;
}

//constexpr lsTime lsTime::Zero;
__declspec(selectany) const lsTime lsTime::Zero;

