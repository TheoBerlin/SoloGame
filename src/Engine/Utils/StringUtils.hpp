#pragma once

constexpr size_t HashString(const char* pInput)
{
	size_t hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;
	constexpr const size_t prime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;

	while (*pInput) {
		hash ^= static_cast<size_t>(*pInput);
		hash *= prime;
		++pInput;
	}

	return hash;
}

template <typename ... Args>
inline std::string FormatString(const std::string& format, Args&& ... args)
{
    // Create a char buffer to hold the formatted string and a null termination
    size_t size = (size_t)snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    std::unique_ptr<char[]> buffer(DBG_NEW char[size]);

    // Format string
    snprintf(buffer.get(), size, format.c_str(), args ...);

    // Convert to std::string without null termination
    return std::string(buffer.get(), buffer.get() + size - 1);
}
