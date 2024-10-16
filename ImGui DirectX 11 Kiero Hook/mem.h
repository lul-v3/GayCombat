#pragma once

#include <cstdint>
#include <vector>

namespace mem
{
	uintptr_t findeAddress(uintptr_t ptr, std::vector<unsigned int> offsets)
	{
		uintptr_t address = ptr;
		for (unsigned int i = 0; i < offsets.size(); ++i)
		{
			address = *(uintptr_t*)address;
			address += offsets[i];
		}

		return address;
	}
}