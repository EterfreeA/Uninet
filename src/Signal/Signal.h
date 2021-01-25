#pragma once

#include <csignal>
#include <functional>
#include <cstdint>
#include <unordered_map>

#ifndef ETERFREE_BEGIN
#define ETERFREE_BEGIN namespace eterfree {
#endif

#ifndef ETERFREE_END
#define ETERFREE_END }
#endif

ETERFREE_BEGIN

class Signal final
{
public:
	using handler_type = decltype(SIG_DFL);
	using functor_type = std::function<void(int)>;

private:
	using mappings_type = std::unordered_map<uint_least64_t, functor_type>;

	static std::unordered_map<int, handler_type> strategies;
	static std::unordered_map<int, mappings_type> mapping_table;

public:
	Signal() = delete;

	static handler_type get(int signal)
	{
		auto iterator = strategies.find(signal);
		return iterator != strategies.end() ? iterator->second : SIG_DFL;
	}

	static void set(int signal, handler_type strategy)
	{
		strategies[signal] = strategy;
	}

	static void insert(int signal, uint_least64_t id, functor_type&& functor);
	static void erase(int signal, uint_least64_t id);

private:
	static void handle(int signal);
};

#ifndef SIGNAL_BEGIN
#define SIGNAL_BEGIN namespace signal {
#endif

#ifndef SIGNAL_END
#define SIGNAL_END }
#endif

SIGNAL_BEGIN
using handler_type = decltype(SIG_DFL);
using functor_type = std::function<void(int)>;

handler_type get(int signal);
void set(int signal, handler_type strategy);
void insert(int signal, uint_least64_t id, functor_type&& functor);
void erase(int signal, uint_least64_t id);
SIGNAL_END

ETERFREE_END
