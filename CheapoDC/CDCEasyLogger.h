// ******************************************************************
// Streaming operator for serial print use to make nice logging like
// LOG_DEBUG("TST", "var1" << var1 << ", var2=" << var2)
// Source code copied from: https://github.com/x821938/EasyLogger
// ******************************************************************
// Source code copied from a project by Alex Skov Jensen
// Details, original source and licensing found here:
//           https://docs.arduino.cc/libraries/easylogger/      
//           https://github.com/x821938/EasyLogger
// 
#ifndef _EASYLOGGGER_h
#define _EASYLOGGGER_h
    #include <Arduino.h>

    // Streaming operator for serial print use to make nice logging like
    // LOG_DEBUG("TST", "var1" << var1 << ", var2=" << var2)
    template <class T>
    inline Print &operator<<(Print &obj, T arg)
    {
        obj.print(arg);
        return obj;
    }
    #define ENDL "\r\n"
    

    // Default to log to "Serial". But it can be defined in code to log to Serial2 for example.
    #ifndef LOG_OUTPUT
        #define LOG_OUTPUT Serial
    #endif

    // LOG levels order/priority. IF LOG_LEVEL_NONE is set, then nothing will be logged
    #define LOG_LEVEL_NONE -1
    #define LOG_LEVEL_EMERGENCY 0
    #define LOG_LEVEL_ALERT 1
    #define LOG_LEVEL_CRITICAL 2
    #define LOG_LEVEL_ERROR 3
    #define LOG_LEVEL_WARNING 4
    #define LOG_LEVEL_NOTICE 5
    #define LOG_LEVEL_INFO 6
    #define LOG_LEVEL_DEBUG 7

    // Default LOG_LEVEL is set to LOG_LEVEL_DEBUG, if it's not defined in user code.
    #ifndef LOG_LEVEL
        #define LOG_LEVEL LOG_LEVEL_DEBUG
    #endif

    // Log formats
    #define LOG_FORMATTING_MILLIS 0
    #define LOG_FORMATTING_HMS 1
    #define LOG_FORMATTING_NOTIME 2

    // Default log formatting is LOG_FORMATTING_HMS, if it's not defined in user code.
    #ifndef LOG_FORMATTING
        #define LOG_FORMATTING LOG_FORMATTING_HMS
    #endif

    #if LOG_LEVEL > LOG_LEVEL_NONE

        // Internal logging function that will be wrapped by definitions like LOG_NOTICE, LOG_ALERT, LOG_CRITICAL etc.
        // Prints time, loglevel and service. No endline. That will be streamed by macro-wrapper
        static void print_log_line_header(uint8_t loglevel, const char *svc)
        {
            // Keep things static to avoid memory fragmentation
            static const char *loglevels_text[] = {"EMERGENCY", "ALERT    ", "CRITICAL ", "ERROR    ", "WARNING  ", "NOTIC    ", "INFO     ", "DEBUG    "};
            static long logTime;
            static char logFormattedTime[18];

            logTime = millis();

            #if LOG_FORMATTING == LOG_FORMATTING_HMS
                static long seconds, minutes, hours, days; // avoid memory fragmentation.
                seconds = logTime / 1000;
                minutes = seconds / 60;
                hours = minutes / 60;
                days = hours / 24;
                sprintf(logFormattedTime, "%03u:%02u:%02u:%02u:%03u", days, hours % 24, minutes % 60, seconds % 60, logTime % 1000);
                LOG_OUTPUT << logFormattedTime << "  ";
            #elif LOG_FORMATTING == LOG_FORMATTING_MILLIS
                sprintf(logFormattedTime, "%09u", logTime);
                LOG_OUTPUT << logFormattedTime << "  ";
            #elif LOG_FORMATTING == LOG_FORMATTING_NOTIME
                // Dont print anything
            #endif
            LOG_OUTPUT << loglevels_text[loglevel] << " [" << svc << "] : ";
        }

        #ifdef LOG_FILTER
            /* If logfiltering is enabled then this function decides if the service should be logged.
               It checks service against LOG_FILTER. Depending on LOG_FILTER_EXCLUDE is set, this
               function returns true or false, showing if it should be logged
               It's compiled out if not defined in user code */
            static bool should_log_line(const char *svc)
            {
                #ifdef LOG_FILTER_EXCLUDE
                    static const bool log_filter_exclude = true;
                #else
                    static const bool log_filter_exclude = false;
                #endif

                bool svc_found_in_filter = (strstr(LOG_FILTER, svc) != NULL);
                if (svc_found_in_filter && !log_filter_exclude) return (true);
                if (!svc_found_in_filter && log_filter_exclude) return (true);
                return (false);
            }
        #endif

    #endif

    /* Reusable code for each loglevel macro function. If filtering is enabled this will be
       inserted at each log-statement. First it's checked if line should be logged according
       to the filter. Then it logs if if it's in the filter
       It will log time-stamp and log-content with an endline */
    #define LOG_RAW_FILTER_LINE(loglevel, svc, content)     \
    {                                                       \
        if (should_log_line(svc))                           \
        {                                                   \
            print_log_line_header(loglevel, svc);           \
            LOG_OUTPUT << content;                          \
            LOG_OUTPUT << ENDL;                             \
        }                                                   \
    }

    /* Reusable code for each loglevel macro funcion. This code is inserted in the log-statement
       if filtering is disabled. Just prints a time-stamp and the log content with an endline */
    #define LOG_RAW_LINE(loglevel, svc, content)            \
    {                                                       \
        print_log_line_header(loglevel, svc);               \
        LOG_OUTPUT << content;                              \
        LOG_OUTPUT << ENDL;                                 \
    }

    /* If the log statement function shouldn't be compiled in, because our loglevel is too low .
       Then no code should be compiled */
    #define LOG_RAW_DO_NOTHING(loglevel, svc, content)

    // Define logging macro functions per LOG_LEVEL

    #if LOG_LEVEL >= LOG_LEVEL_DEBUG
        #ifdef LOG_FILTER
            #define LOG_DEBUG(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_DEBUG, svc, content)
        #else
            #define LOG_DEBUG(svc, content) LOG_RAW_LINE(LOG_LEVEL_DEBUG, svc, content)
        #endif
    #else
        #define LOG_DEBUG(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_DEBUG, svc, content)
    #endif

    #if LOG_LEVEL >= LOG_LEVEL_INFO
        #ifdef LOG_FILTER
            #define LOG_INFO(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_INFO, svc, content)
        #else
            #define LOG_INFO(svc, content) LOG_RAW_LINE(LOG_LEVEL_INFO, svc, content)
        #endif
    #else
        #define LOG_INFO(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_INFO, svc, content)
    #endif

    #if LOG_LEVEL >= LOG_LEVEL_NOTICE
        #ifdef LOG_FILTER
            #define LOG_NOTICE(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_NOTICE, svc, content)
        #else
            #define LOG_NOTICE(svc, content) LOG_RAW_LINE(LOG_LEVEL_NOTICE, svc, content)
        #endif
    #else
        #define LOG_NOTICE(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_NOTICE, svc, content)
    #endif

    #if LOG_LEVEL >= LOG_LEVEL_WARNING
        #ifdef LOG_FILTER
            #define LOG_WARNING(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_WARNING, svc, content)
        #else
            #define LOG_WARNING(svc, content) LOG_RAW_LINE(LOG_LEVEL_WARNING, svc, content)
        #endif
    #else
        #define LOG_WARNING(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_WARNING, svc, content)
    #endif

    #if LOG_LEVEL >= LOG_LEVEL_ERROR
        #ifdef LOG_FILTER
            #define LOG_ERROR(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_ERROR, svc, content)
        #else
            #define LOG_ERROR(svc, content) LOG_RAW_LINE(LOG_LEVEL_ERROR, svc, content)
        #endif
    #else
        #define LOG_ERROR(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_ERROR, svc, content)
    #endif

    #if LOG_LEVEL >= LOG_LEVEL_CRITICAL
        #ifdef LOG_FILTER
            #define LOG_CRITICAL(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_CRITICAL, svc, content)
        #else
            #define LOG_CRITICAL(svc, content) LOG_RAW_LINE(LOG_LEVEL_CRITICAL, svc, content)
        #endif
    #else
        #define LOG_CRITICAL(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_CRITICAL, svc, content)
    #endif

    #if LOG_LEVEL >= LOG_LEVEL_ALERT
        #ifdef LOG_FILTER
            #define LOG_ALERT(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_ALERT, svc, content)
        #else
            #define LOG_ALERT(svc, content) LOG_RAW_LINE(LOG_LEVEL_ALERT, svc, content)
        #endif
    #else
        #define LOG_ALERT(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_ALERT, svc, content)
    #endif

    #if LOG_LEVEL >= LOG_LEVEL_EMERGENCY
        #ifdef LOG_FILTER
            #define LOG_EMERGENCY(svc, content) LOG_RAW_FILTER_LINE(LOG_LEVEL_EMERGENCY, svc, content)
        #else
            #define LOG_EMERGENCY(svc, content) LOG_RAW_LINE(LOG_LEVEL_EMERGENCY, svc, content)
        #endif
    #else
        #define LOG_EMERGENCY(svc, content) LOG_RAW_DO_NOTHING(LOG_LEVEL_EMERGENCY, svc, content)
    #endif

#endif // ifndef _EASYLOGGGER_h
