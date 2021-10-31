#include "SessionManager.hpp"

#include <string>
#include <iostream>

NETWORK_BEGIN

SessionManager::Proxy::~Proxy()
{
	_manager.onPush(getIndex());
}

void SessionManager::onPush(const Index& _index)
{
	std::lock_guard lock(_queueMutex);
	auto iterator = _mappings.find(_index);
	if (iterator == _mappings.end())
		return;

	auto& stream = iterator->second->getInputStream();
	if (!stream.empty())
		_queue.emplace_back(*iterator);
	else
		_mappings.erase(iterator);
}

void SessionManager::onPop(const Index& _index)
{
	std::lock_guard lock(_queueMutex);
	_mappings.erase(_index);
	_queue.remove_if([&_index](const SessionPair& _pair) { return _pair.first == _index; });
}

bool SessionManager::onConnect(const Index& _index, std::shared_ptr<Session> _session)
{
	if (_session == nullptr)
		return false;

	auto id = _session->getID();
	std::lock_guard lock(_sessionMutex);

	if (id)
	{
		_idMap.erase(id.value());
		_idMap.emplace(id.value(), _session);
	}

	_sessions.erase(_index);
	return _sessions.emplace(_index, _session).second;
}

void SessionManager::onDisconnect(const Index& _index)
{
	std::lock_guard lock(_sessionMutex);
	auto iterator = _sessions.find(_index);
	if (iterator == _sessions.end())
		return;

	if (auto id = iterator->second->getID())
		_idMap.erase(id.value());
	_sessions.erase(iterator);
	onPop(_index);
}

void SessionManager::onReceive(const Index& _index, const char* _data, SizeType _size)
{
	std::cout << std::string(_data, _size) << std::endl;

	std::unique_lock lock(_sessionMutex);
	auto iterator = _sessions.find(_index);
	if (iterator == _sessions.end())
		return;

	auto& session = iterator->second;
	auto& stream = session->getInputStream();
	stream.push(_data, _size);

	if (!stream.empty())
	{
		std::lock_guard lock(_queueMutex);
		if (_mappings.find(_index) == _mappings.end())
		{
			_mappings.emplace(_index, session);
			_queue.emplace_back(_index, session);
		}
	}
}

void SessionManager::onSend(const Index& _index, SizeType _size, SendFunctor& _functor)
{
	std::unique_lock lock(_sessionMutex);
	auto iterator = _sessions.find(_index);
	if (iterator == _sessions.end())
		return;

	auto session = iterator->second;
	lock.unlock();

	auto& stream = session->getOutputStream();
	auto proxy = stream.pop();
	if (proxy->empty())
		return;

	auto data = proxy->data(_size);
	if (auto length = _functor(_index._pair.first, data, static_cast<int>(_size)); length > 0)
		proxy->pop(static_cast<SizeType>(length));
}

void SessionManager::clear()
{
	std::lock_guard sessionLock(_sessionMutex);
	_idMap.clear();
	_sessions.clear();

	std::lock_guard queueLock(_queueMutex);
	_mappings.clear();
	_queue.clear();
}

std::unique_ptr<SessionManager::Proxy> SessionManager::dispatch()
{
	std::lock_guard lock(_queueMutex);
	if (_queue.empty())
		return nullptr;

	auto _pair = _queue.front();
	_queue.pop_front();
	return std::make_unique<Proxy>(*this, _pair);
}

NETWORK_END
