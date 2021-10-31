#pragma once

template <typename _Type>
class Singleton
{
public:
	static _Type& getInstance()
	{
		static _Type instance;
		return instance;
	}
};
