//
// Created by monty on 26/09/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_LOGGER_H
#define DUNGEONSOFNOUDAR_NDK_LOGGER_H

namespace odb {
	class Logger {
	public:
		static void log(const char* format, ...);
        static void log(std::string format, ...);
	};
}

#endif //DUNGEONSOFNOUDAR_NDK_LOGGER_H
