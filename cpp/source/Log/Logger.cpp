#include "Log/Logger.h"

#include <utility>
#include <tuple>
#include <cctype>
#include <deque>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <atomic>
#include <thread>
#include <mutex>

ETERFREE_SPACE_BEGIN

class CommonLogger : public Logger
{
public:
	using TupleType = std::tuple<TimePoint, \
		ThreadID, LEVEL, std::source_location, Functor>;

protected:
	static bool getPath(std::filesystem::path& _left, \
		const std::string& _right) noexcept;

	static bool getPath(std::filesystem::path& _path, \
		const std::string& _prefix, const std::string& _suffix) noexcept;

	static bool getPath(std::filesystem::path& _path, \
		const File& _file, TimeType _time = 0) noexcept;

	static SizeType getSize(const std::string& _path) noexcept;

	static TimeType getTime(const std::string& _name, \
		const std::string& _prefix, const std::string& _suffix) noexcept;

	static bool existDirectory(const std::filesystem::path& _path) noexcept;

	static bool createDirectory(const std::string& _path) noexcept;

	static bool existFile(const std::filesystem::path& _path) noexcept;

	static bool getFile(File& _left, const File& _right) noexcept;

	static bool moveFile(const std::filesystem::path& _source, \
		const std::filesystem::path& _target) noexcept;

	static bool moveFile(const File& _source, \
		const File& _target) noexcept;

	static void adjustFile(const File& _file, \
		const Limit& _limit, bool _moved = false) noexcept;

public:
	static TimeType getTime(const TimePoint& _timePoint) noexcept;
};

class SingleThreadLogger : public CommonLogger
{
	Limit _limit;
	SizeType _size;

	File _file;
	std::ofstream _stream;

	FlagType _flag;
	std::deque<TupleType> _queue;

public:
	static std::shared_ptr<Logger> get() noexcept;

public:
	virtual ~SingleThreadLogger() noexcept { output(); }

	virtual FlagType getFlag() const noexcept override { return _flag; };

	virtual void setFlag(FlagType _flag) noexcept override
	{
		this->_flag = _flag;
	}

	virtual Limit getLimit() const noexcept override
	{
		return _limit;
	}

	virtual void setLimit(const Limit& _limit) noexcept override;

	virtual bool getFile(File& _file) const noexcept override
	{
		return CommonLogger::getFile(_file, this->_file);
	}

	virtual bool setFile(const File& _file) noexcept override;

	virtual void input(LEVEL _level, \
		const std::source_location& _location, \
		const Functor& _functor) noexcept override;

	virtual void execute() noexcept override { output(); }

private:
	bool isLimited(SizeType _size) noexcept;

	bool openStream() noexcept;

	void closeStream() noexcept;

	bool updateStream(const Limit& _limit) noexcept;

	void output() noexcept;

public:
	SingleThreadLogger()
		: _size(0), _flag(FLAG::NONE) {}
};

class MultiThreadLogger : public CommonLogger
{
	mutable std::mutex _uniqueMutex;
	std::atomic<Limit> _limit;
	SizeType _size;

	File _file;
	std::ofstream _stream;

	std::mutex _queueMutex;
	std::deque<TupleType> _queue;

public:
	static std::shared_ptr<Logger> get() noexcept;

public:
	virtual ~MultiThreadLogger() noexcept { output(); }

	virtual FlagType getFlag() const noexcept override { return FLAG::NONE; };

	virtual void setFlag(FlagType) noexcept override {};

	virtual Limit getLimit() const noexcept override
	{
		return _limit.load(std::memory_order::relaxed);
	}

	virtual void setLimit(const Limit& _limit) noexcept override;

	virtual bool getFile(File& _file) const noexcept override;

	virtual bool setFile(const File& _file) noexcept override;

	virtual void input(LEVEL _level, \
		const std::source_location& _location, \
		const Functor& _functor) noexcept override;

	virtual void execute() noexcept override { output(); }

private:
	bool openStream() noexcept;

	void closeStream() noexcept;

	bool updateStream(const Limit& _limit) noexcept;

	void output() noexcept;

public:
	MultiThreadLogger() : _size(0) {}
};

bool operator==(const Logger::File& _left, \
	const Logger::File& _right) noexcept
{
	if (&_left == &_right) return true;

	return _left._path == _right._path \
		&& _left._prefix == _right._prefix \
		&& _left._suffix == _right._suffix;
}

std::shared_ptr<Logger> Logger::get(MODE _mode) noexcept
{
	switch (_mode)
	{
	case MODE::SINGLE_THREAD:
		return SingleThreadLogger::get();
	case MODE::MULTI_THREAD:
		return MultiThreadLogger::get();
	default:
		try
		{
			std::ostringstream stream;
			stream << "unknown mode " << static_cast<SizeType>(_mode);
			output(LEVEL::ERROR, std::source_location::current(), stream.str());
		}
		catch (std::exception& exception)
		{
			output(std::source_location::current(), exception);
		}
	}
	return nullptr;
}

void Logger::output(LEVEL _level, \
	const std::source_location& _location, \
	const char* _description) noexcept
{
	try
	{
		std::ostringstream stream;
		stream << std::chrono::system_clock::now() \
			<< ThreadID(std::this_thread::get_id()) \
			<< _level << _location \
			<< _description << std::endl;

		if (_level < LEVEL::WARN)
			std::cout << stream.str();
		else
			std::cerr << stream.str();
	}
	catch (std::exception&) {}
}

void Logger::output(LEVEL _level, \
	const std::source_location& _location, \
	const std::string& _description) noexcept
{
	try
	{
		std::ostringstream stream;
		stream << std::chrono::system_clock::now() \
			<< ThreadID(std::this_thread::get_id()) \
			<< _level << _location \
			<< _description << std::endl;

		if (_level < LEVEL::WARN)
			std::cout << stream.str();
		else
			std::cerr << stream.str();
	}
	catch (std::exception&) {}
}

void Logger::output(const std::source_location& _location, \
	const std::exception& _exception) noexcept
{
	try
	{
		std::ostringstream stream;
		stream << std::chrono::system_clock::now() \
			<< ThreadID(std::this_thread::get_id()) \
			<< LEVEL::ERROR << _location \
			<< _exception.what() << std::endl;
		std::cerr << stream.str();
	}
	catch (std::exception&) {}
}

void Logger::output(const std::source_location& _location, \
	const std::error_code& _code) noexcept
{
	try
	{
		std::ostringstream stream;
		stream << std::chrono::system_clock::now() \
			<< ThreadID(std::this_thread::get_id()) \
			<< LEVEL::ERROR << _location \
			<< _code.message() << std::endl;
		std::cerr << stream.str();
	}
	catch (std::exception&) {}
}

std::ostream& operator<<(std::ostream& _stream, \
	const Logger::TimePoint& _timePoint)
{
	return _stream << '[' << CommonLogger::getTime(_timePoint) << ']';
}

static const char* LOG_LEVEL[] =
{
	"", "RUN", "DEBUG", "WARN", "ERROR"
};

std::ostream& operator<<(std::ostream& _stream, Logger::LEVEL _level)
{
	if (_level > Logger::LEVEL::EMPTY)
	{
		auto level = static_cast<std::uint8_t>(_level);
		if (level < size(LOG_LEVEL))
			_stream << '[' << LOG_LEVEL[level] << ']';
	}
	return _stream;
}

bool CommonLogger::getPath(std::filesystem::path& _left, \
	const std::string& _right) noexcept
{
	try
	{
		if (_right.empty()) _left = ".";
		else
		{
			_left = _right;
			_left.make_preferred();
		}
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		_left.clear();
		return false;
	}
	return true;
}

bool CommonLogger::getPath(std::filesystem::path& _path, \
	const std::string& _prefix, const std::string& _suffix) noexcept
{
	try
	{
		_path.append(_prefix).concat(_suffix);
		_path.make_preferred();
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		_path.clear();
		return false;
	}
	return true;
}

bool CommonLogger::getPath(std::filesystem::path& _path, \
	const File& _file, TimeType _time) noexcept
{
	try
	{
		_path = _file._path;
		_path.append(_file._prefix);
		if (_time != 0)
			_path.concat(std::to_string(_time));
		_path.concat(_file._suffix);
		_path.make_preferred();
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		_path.clear();
		return false;
	}
	return true;
}

CommonLogger::SizeType CommonLogger::getSize(const std::string& _path) noexcept
{
	try
	{
		std::filesystem::directory_entry entry(_path);
		if (entry.exists() && entry.is_regular_file())
			return entry.file_size();
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
	}
	return 0;
}

CommonLogger::TimeType CommonLogger::getTime(const std::string& _name, \
	const std::string& _prefix, const std::string& _suffix) noexcept
{
	if (!_name.starts_with(_prefix) || !_name.ends_with(_suffix))
		return 0;

	auto begin = _prefix.size();
	auto end = _name.size() - _suffix.size();
	if (begin >= end) return 0;

	try
	{
		auto string = _name.substr(begin, end - begin);
		for (auto character : string)
			if (!std::isdigit(character)) return 0;
		return std::stoll(string);
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
	}
	return 0;
}

bool CommonLogger::existDirectory(const std::filesystem::path& _path) noexcept
{
	try
	{
		std::filesystem::directory_entry entry(_path);
		return entry.exists() && entry.is_directory();
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
	}
	return false;
}

bool CommonLogger::createDirectory(const std::string& _path) noexcept
{
	using namespace std::filesystem;
	path dirPath;
	if (!getPath(dirPath, _path)) return false;

	try
	{
		directory_entry entry(dirPath);
		if (entry.exists() && entry.is_directory()) return true;

		if (!create_directories(entry)) return false;
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		return false;
	}
	return true;
}

bool CommonLogger::existFile(const std::filesystem::path& _path) noexcept
{
	try
	{
		std::filesystem::directory_entry entry(_path);
		return entry.exists() && entry.is_regular_file();
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
	}
	return false;
}

bool CommonLogger::getFile(File& _left, const File& _right) noexcept
{
	try
	{
		_left = _right;
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		return false;
	}
	return true;
}

bool CommonLogger::moveFile(const std::filesystem::path& _source, \
	const std::filesystem::path& _target) noexcept
{
	try
	{
		rename(_source, _target);
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		return false;
	}
	return true;
}

bool CommonLogger::moveFile(const File& _source, \
	const File& _target) noexcept
{
	using namespace std::filesystem;
	path oldPath;
	if (!getPath(oldPath, _source._path))
		return false;

	try
	{
		directory_entry entry(oldPath);
		if (!entry.exists() || !entry.is_directory())
			return true;
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		return false;
	}

	if (!createDirectory(_target._path))
		return false;

	bool result = true;
	try
	{
		for (const auto& entry : directory_iterator(oldPath))
		{
			if (!entry.is_regular_file()) continue;

			auto time = getTime(entry.path().filename().string(), \
				_source._prefix, _source._suffix);
			if (time == 0) continue;

			path newPath;
			if (!getPath(newPath, _target, time))
			{
				result = false;
				continue;
			}

			if (!moveFile(entry.path(), newPath))
				result = false;
		}
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		return false;
	}

	if (!getPath(oldPath, \
		_source._prefix, _source._suffix))
		return false;

	try
	{
		directory_entry entry(oldPath);
		if (entry.exists() && entry.is_regular_file())
		{
			path newPath;
			if (!getPath(newPath, _target))
				result = false;
			else if (!moveFile(entry.path(), newPath))
				result = false;
		}
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
		return false;
	}
	return result;
}

void CommonLogger::adjustFile(const File& _file, \
	const Limit& _limit, bool _moved) noexcept
{
	using namespace std::filesystem;
	path oldPath;
	if (!_moved && getPath(oldPath, _file) \
		&& existFile(oldPath))
	{
		path newPath;
		auto time = getTime(std::chrono::system_clock::now());
		if (getPath(newPath, _file, time))
			moveFile(oldPath, newPath);
	}

	if (_limit._quantity <= 0 \
		|| !getPath(oldPath, _file._path) \
		|| !existDirectory(oldPath)) return;

	try
	{
		std::map<TimeType, path> mapping;
		for (const auto& entry : directory_iterator(oldPath))
		{
			if (!entry.is_regular_file()) continue;

			auto time = getTime(entry.path().filename().string(), \
				_file._prefix, _file._suffix);
			if (time == 0) continue;

			mapping.emplace(time, entry.path());
		}

		if (mapping.size() < _limit._quantity)
			return;

		auto iterator = mapping.cbegin();
		auto size = mapping.size() - _limit._quantity + 1;
		for (decltype(size) index = 0; \
			index < size; ++index, ++iterator)
			if (std::error_code code; \
				!remove(iterator->second, code))
				output(std::source_location::current(), code);
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
	}
}

CommonLogger::TimeType CommonLogger::getTime(const TimePoint& _timePoint) noexcept
{
	try
	{
		using namespace std::chrono;
		auto duration = _timePoint.time_since_epoch();
		return duration_cast<nanoseconds>(duration).count();
	}
	catch (std::exception& exception)
	{
		output(std::source_location::current(), exception);
	}
	return 0;
}

std::shared_ptr<Logger> SingleThreadLogger::get() noexcept
{
	try
	{
		return std::make_shared<SingleThreadLogger>();
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
	return nullptr;
}

void SingleThreadLogger::setLimit(const Limit& _limit) noexcept
{
	auto size = this->_limit._size;
	this->_limit = _limit;
	if (_limit._size <= 0 || size > 0) return;

	try
	{
		std::filesystem::path path;
		if (getPath(path, _file))
			_size = getSize(path.string());
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

bool SingleThreadLogger::setFile(const File& _file) noexcept
{
	if (_file.empty()) return false;

	if (this->_file == _file) return true;

	closeStream();

	if (!this->_file.empty() \
		&& !moveFile(this->_file, _file))
	{
		openStream();
		return false;
	}

	if (!CommonLogger::getFile(this->_file, _file))
		return false;

	adjustFile(_file, _limit, true);

	return openStream();
}

void SingleThreadLogger::input(LEVEL _level, \
	const std::source_location& _location, \
	const Functor& _functor) noexcept
{
	auto time = std::chrono::system_clock::now();
	auto id = ThreadID(std::this_thread::get_id());

	try
	{
		if ((_flag & FLAG::ASYNC) != 0)
		{
			auto tuple = std::make_tuple(time, \
				id, _level, _location, _functor);
			_queue.push_back(std::move(tuple));
			return;
		}
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
		return;
	}

	try
	{
		if (!_stream.is_open()) return;
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
		return;
	}

	try
	{
		if (_limit._size <= 0)
		{
			std::string description;
			_stream << time << id << _level << _location \
				<< _functor(description) << std::endl;
			return;
		}
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
		return;
	}

	try
	{
		std::ostringstream stream;
		std::string description;
		stream << time << id << _level << _location \
			<< _functor(description) << std::endl;

		auto log = stream.str();
		if (isLimited(log.size()) \
			&& !updateStream(_limit)) return;

		_size += log.size();
		_stream << log << std::flush;
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

bool SingleThreadLogger::isLimited(SizeType _size) noexcept
{
	return this->_size >= _limit._size \
		|| this->_size > 0 \
		&& _size > _limit._size - this->_size;
}

bool SingleThreadLogger::openStream() noexcept
{
	if (!createDirectory(_file._path))
		return false;

	std::filesystem::path filePath;
	if (!getPath(filePath, _file))
		return false;

	try
	{
		_size = getSize(filePath.string());

		using StreamType = decltype(_stream);
		_stream.open(filePath, StreamType::app);
		return _stream.is_open();
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
	return false;
}

void SingleThreadLogger::closeStream() noexcept
{
	try
	{
		if (_stream.is_open()) _stream.close();
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

bool SingleThreadLogger::updateStream(const Limit& _limit) noexcept
{
	closeStream();
	adjustFile(_file, _limit);
	return openStream();
}

void SingleThreadLogger::output() noexcept
{
	if (_queue.empty()) return;

	try
	{
		decltype(_queue) queue;
		queue.swap(_queue);

		if (!_stream.is_open()) return;

		if (_limit._size <= 0)
		{
			for (const auto& [time, id, level, location, functor] : queue)
			{
				std::string description;
				_stream << time << id << level << location \
					<< functor(description) << std::endl;
			}
			return;
		}

		for (const auto& [time, id, level, location, functor] : queue)
		{
			std::ostringstream stream;
			std::string description;
			stream << time << id << level << location \
				<< functor(description) << std::endl;

			auto log = stream.str();
			if (isLimited(log.size()) \
				&& !updateStream(_limit)) return;

			_size += log.size();
			_stream << log;
		}
		_stream << std::flush;
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

std::shared_ptr<Logger> MultiThreadLogger::get() noexcept
{
	try
	{
		static std::mutex mutex;
		std::lock_guard lock(mutex);

		static std::shared_ptr<MultiThreadLogger> logger;
		if (!logger)
			logger = std::make_shared<MultiThreadLogger>();
		return logger;
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
	return nullptr;
}

void MultiThreadLogger::setLimit(const Limit& _limit) noexcept
{
	auto limit = this->_limit.exchange(_limit, \
		std::memory_order::relaxed);
	if (_limit._size <= 0 || limit._size > 0) return;

	try
	{
		std::lock_guard lock(_uniqueMutex);
		std::filesystem::path path;
		if (getPath(path, _file))
			_size = getSize(path.string());
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

bool MultiThreadLogger::getFile(File& _file) const noexcept
{
	try
	{
		std::lock_guard lock(_uniqueMutex);
		return CommonLogger::getFile(_file, this->_file);
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
	return false;
}

bool MultiThreadLogger::setFile(const File& _file) noexcept
{
	if (_file.empty()) return false;

	try
	{
		std::lock_guard lock(_uniqueMutex);

		if (this->_file == _file) return true;

		closeStream();

		if (!this->_file.empty() \
			&& !moveFile(this->_file, _file))
		{
			openStream();
			return false;
		}

		if (!CommonLogger::getFile(this->_file, _file))
			return false;

		auto limit = _limit.load(std::memory_order::relaxed);
		adjustFile(_file, limit, true);

		return openStream();
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
	return false;
}

void MultiThreadLogger::input(LEVEL _level, \
	const std::source_location& _location, \
	const Functor& _functor) noexcept
{
	auto time = std::chrono::system_clock::now();
	auto id = ThreadID(std::this_thread::get_id());

	try
	{
		auto tuple = std::make_tuple(time, \
			id, _level, _location, _functor);

		std::lock_guard lock(_queueMutex);
		_queue.push_back(std::move(tuple));
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

bool MultiThreadLogger::openStream() noexcept
{
	if (!createDirectory(_file._path))
		return false;

	std::filesystem::path filePath;
	if (!getPath(filePath, _file))
		return false;

	try
	{
		_size = getSize(filePath.string());

		using StreamType = decltype(_stream);
		_stream.open(filePath, StreamType::app);
		return _stream.is_open();
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
	return false;
}

void MultiThreadLogger::closeStream() noexcept
{
	try
	{
		if (_stream.is_open()) _stream.close();
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

bool MultiThreadLogger::updateStream(const Limit& _limit) noexcept
{
	closeStream();
	adjustFile(_file, _limit);
	return openStream();
}

void MultiThreadLogger::output() noexcept
{
	try
	{
		std::lock_guard uniqueLock(_uniqueMutex);

		std::unique_lock queueLock(_queueMutex);
		if (_queue.empty()) return;

		decltype(_queue) queue;
		queue.swap(_queue);
		queueLock.unlock();

		if (!_stream.is_open()) return;

		auto limit = _limit.load(std::memory_order::relaxed);
		if (limit._size <= 0)
		{
			for (const auto& [time, id, level, location, functor] : queue)
			{
				std::string description;
				_stream << time << id << level << location \
					<< functor(description) << std::endl;
			}
			return;
		}

		for (const auto& [time, id, level, location, functor] : queue)
		{
			std::ostringstream stream;
			std::string description;
			stream << time << id << level << location \
				<< functor(description) << std::endl;

			auto log = stream.str();
			if (_size >= limit._size \
				|| _size > 0 \
				&& log.size() > limit._size - _size)
				if (!updateStream(limit)) return;

			_size += log.size();
			_stream << log;
		}
		_stream << std::flush;
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
}

ETERFREE_SPACE_END
