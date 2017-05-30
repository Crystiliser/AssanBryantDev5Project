#pragma once

#ifdef LOADERDEFINE
#define LOADERAPI __declspec(dllexport)
#else
#define LOADERAPI __declspec(dllimport)
#endif // LOADERDEFINE

namespace functionLibrary
{
	class functions
	{
	public:
		static LOADERAPI int add(int a, int b);
	};
}