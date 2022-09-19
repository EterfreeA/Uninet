#pragma once

#include <utility>
#include <source_location>
#include <exception>
#include <thread>

#include "Core.hpp"
#include "Core/DesignPattern/Singleton.hpp"
#include "Log/Logger.h"

ETERFREE_SPACE_BEGIN

class Framework
{
	friend Framework& Singleton<Framework>::get();

private:
	Framework() = default;
};

template <typename _Type, typename... _Args>
_Type* make(_Args&&... _args) noexcept
{
	_Type* pointer = nullptr;
	try
	{
		pointer = new _Type(std::forward<_Args>(_args)...);
	}
	catch (std::exception& exception)
	{
		Logger::output(std::source_location::current(), exception);
	}
	return pointer;
}

ETERFREE_SPACE_END
