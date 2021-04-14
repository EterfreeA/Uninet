#include "Signal.hpp"

#include <utility>
#include <exception>
#include <iostream>

ETERFREE_BEGIN

std::unordered_map<int, void(*)(int)> Signal::strategies;
std::unordered_map<int, Signal::mappings_type> Signal::mapping_table;

void Signal::insert(int signal, uint_least64_t id, functor_type&& functor)
{
	if (functor == nullptr)
		return;

	using std::move;
	auto iterator = mapping_table.find(signal);
	if (iterator != mapping_table.end())
	{
		iterator->second[id] = move(functor);
		return;
	}

	mapping_table[signal] = move(mappings_type{ std::make_pair(id, move(functor)) });
	std::signal(signal, handle);
}

void Signal::erase(int signal, uint_least64_t id)
{
	auto iterator = mapping_table.find(signal);
	if (iterator == mapping_table.end())
		return;

	auto& mappings = iterator->second;
	if (auto iterator = mappings.find(id); iterator == mappings.end())
		return;
	else
		mappings.erase(iterator);

	if (mappings.empty())
	{
		std::signal(signal, get(signal));
		mapping_table.erase(signal);
	}
}

void Signal::handle(int signal)
{
	auto iterator = mapping_table.find(signal);
	if (iterator != mapping_table.end())
	{
		for (auto& pair : iterator->second)
		{
			try
			{
				pair.second(signal);
			}
			catch (std::exception& exception)
			{
				std::cerr << exception.what() << std::endl;
			}
		}
	}

	std::signal(signal, get(signal));
	std::raise(signal);
	std::signal(signal, handle);
}

SIGNAL_BEGIN

using mappings_type = std::unordered_map<uint_least64_t, functor_type>;
static std::unordered_map<int, handler_type> strategies;
static std::unordered_map<int, mappings_type> mapping_table;

handler_type get(int signal)
{
	auto iterator = strategies.find(signal);
	return iterator != strategies.end() ? iterator->second : SIG_DFL;
}

void set(int signal, handler_type strategy)
{
	strategies[signal] = strategy;
}

static void handle(int signal)
{
	auto iterator = mapping_table.find(signal);
	if (iterator != mapping_table.end())
	{
		for (auto& pair : iterator->second)
		{
			try
			{
				pair.second(signal);
			}
			catch (std::exception& exception)
			{
				std::cerr << exception.what() << std::endl;
			}
		}
	}

	std::signal(signal, get(signal));
	std::raise(signal);
	std::signal(signal, handle);
}

void insert(int signal, uint_least64_t id, functor_type&& functor)
{
	if (functor == nullptr)
		return;

	using std::move;
	auto iterator = mapping_table.find(signal);
	if (iterator != mapping_table.end())
	{
		iterator->second[id] = move(functor);
		return;
	}

	mapping_table[signal] = move(mappings_type{ std::make_pair(id, move(functor)) });
	std::signal(signal, handle);
}

void erase(int signal, uint_least64_t id)
{
	auto iterator = mapping_table.find(signal);
	if (iterator == mapping_table.end())
		return;

	auto& mappings = iterator->second;
	if (auto iterator = mappings.find(id); iterator == mappings.end())
		return;
	else
		mappings.erase(iterator);

	if (mappings.empty())
	{
		std::signal(signal, get(signal));
		mapping_table.erase(signal);
	}
}

SIGNAL_END

ETERFREE_END
