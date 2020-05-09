#pragma once

#define STRINGIFY(x) #x

#define ROCKETVERSION(major, minor)                                                          \
	constexpr unsigned short majorVersion = major;                                           \
	constexpr unsigned short minorVersion = minor;                                           \
	constexpr unsigned int pluginVersionComp = majorVersion << sizeof(short) | minorVersion; \
	static const char* versionString = STRINGIFY(major) "." STRINGIFY(minor);

namespace SuperSonicML::Constants {
	ROCKETVERSION(1, 0);

	static const char* pluginName = "SuperSonicML";
}