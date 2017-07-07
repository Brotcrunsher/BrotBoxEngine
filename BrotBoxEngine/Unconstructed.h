#pragma once


namespace bbe
{
	namespace INTERNAL
	{
		template <typename T>
		union Unconstructed
		{
			//this little hack prevents the constructor of T to be called
			//allows the use of new and its auto alignment features
			T value;

			Unconstructed() {}
			~Unconstructed() {}
		};
	}
}