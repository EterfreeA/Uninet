#pragma once

#include <functional>
#include <chrono>
#include <source_location>
#include <memory>
#include <cstdint>
#include <exception>
#include <system_error>
#include <string>
#include <ostream>

#include "Core.hpp"

ETERFREE_SPACE_BEGIN

class Logger
{
public:
	using FlagType = std::uint8_t;
	using SizeType = std::uintmax_t;
	using Quantity = std::uint32_t;
	using TimeType = std::chrono::nanoseconds::rep;
	using TimePoint = std::chrono::system_clock::time_point;
	using Functor = std::function<std::string&(std::string&)>;

public:
	enum class LEVEL : std::uint8_t
	{ EMPTY, RUN, DEBUG, WARN, ERROR };

	enum class MODE : std::uint8_t
	{ SINGLE_THREAD, MULTI_THREAD };

	enum FLAG : FlagType
	{
		NONE = 0x00,
		ASYNC = 0x01, // 单线程模式默认同步，可以指定异步方式；多线程模式仅异步方式
	};

	struct Limit
	{
		SizeType _size;
		Quantity _quantity;

		Limit() noexcept
			: _size(0), _quantity(0) {}
	};

	struct File
	{
		std::string _path;
		std::string _prefix;
		std::string _suffix;

		friend bool operator==(const File& _left, \
			const File& _right) noexcept;

		bool empty() const noexcept
		{
			return _prefix.empty() \
				&& _suffix.empty();
		}
	};

public:
	static std::shared_ptr<Logger> get(MODE _mode) noexcept;

	static void output(LEVEL _level, \
		const std::source_location& _location, \
		const char* _description) noexcept;

	static void output(LEVEL _level, \
		const std::source_location& _location, \
		const std::string& _description) noexcept;

	static void output(const std::source_location& _location, \
		const std::exception& _exception) noexcept;

	static void output(const std::source_location& _location, \
		const std::error_code& _code) noexcept;

public:
	virtual ~Logger() noexcept {}

	virtual FlagType getFlag() const noexcept = 0;

	virtual void setFlag(FlagType _flag) noexcept = 0;

	virtual Limit getLimit() const noexcept = 0;

	virtual void setLimit(const Limit& _limit) noexcept = 0;

	virtual bool getFile(File& _file) const noexcept = 0;

	virtual bool setFile(const File& _file) noexcept = 0;

	virtual void input(LEVEL _level, \
		const std::source_location& _location, \
		const Functor& _functor) noexcept = 0;

	virtual void execute() noexcept {}
};

std::ostream& operator<<(std::ostream& _stream, \
	const Logger::TimePoint& _timePoint);

std::ostream& operator<<(std::ostream& _stream, \
	Logger::LEVEL _level);

ETERFREE_SPACE_END
