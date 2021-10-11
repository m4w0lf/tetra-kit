#ifndef LOG_H
#define LOG_H
#include <cstdio>
#include <cstdarg>                                                              // for variadic functions

namespace Tetra {

    /**
     * @brief Logging level
     *
     * Note that enum members are numbered to ease tests
     *
     */

    enum LogLevel {
        NONE,
        LOW,
        MEDIUM,
        HIGH,
        VERYHIGH
    };

    /**
     * @brief Screen logger class
     *
     */

    class Log {
    public:
        Log(const LogLevel level);
        ~Log();

        void print(const LogLevel level, const char * fmt, ...);
        LogLevel getLevel();

    private:
        LogLevel m_level;                                                       ///< Minimum log level
    };
};

#endif /* LOG_H */
