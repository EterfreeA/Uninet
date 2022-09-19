#pragma once

template <typename _FriendType>
class Singleton
{
public:
	using FriendType = _FriendType;

public:
	static FriendType& get()
	{
		static FriendType instance;
		return instance;
	}
};
