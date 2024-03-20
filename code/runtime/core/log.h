#ifndef LOG_H
#define LOG_H

enum log_level {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
};

global const string G_LogLevelStr[] = {
    string("Debug"),
    string("Info"),
    string("Warning"),
    string("Error")
};

struct log_event {
    date_time DateTime;
    string    Module;
    log_level Level;
    string    Message;
};

string Log_Event_To_String(allocator* Allocator, log_event* LogEvent);

struct logger {
    u64 ThreadID;

    u32 PoolIndex;
	u32 PoolNextIndex;

	logger* HashNext;
	logger* HashPrev;
};

logger* Logger_Get();
void    Logger_Delete(logger* Logger);

struct logger_pool {
	logger Loggers[MAX_THREAD_COUNT];
	u32    FirstFreeIndex;
	u32    MaxUsed;
};

struct logger_slot {
	logger* SlotHead;
	logger* SlotTail;
};

struct logger_map {
	logger_slot Slots[MAX_THREAD_COUNT];
};

struct log_manager {
    ak_mutex    AllocateLock;
    ak_mutex    MapLock;
    logger_map  LoggerMap;
    logger_pool LoggerPool;
    ak_tls      LoggerLocalStorage;
};

log_manager* Log_Manager_Create();
void         Log_Manager_Delete();
void         Log_Manager_Log(log_level Level, string Module, const char* Format, ...);
log_manager* Log_Manager_Get();
void         Log_Manager_Set(log_manager* Manager);

#if defined(COMPILER_GCC)
#define Log_Debug(module, format, ...) Log_Manager_Log(LOG_LEVEL_DEBUG, module, format, ##__VA_ARGS__)
#define Log_Info(module, format, ...) Log_Manager_Log(LOG_LEVEL_INFO, module, format, ##__VA_ARGS__)
#define Log_Warning(module, format, ...) Log_Manager_Log(LOG_LEVEL_WARNING, module, format, ##__VA_ARGS__)
#define Log_Error(module, format, ...) Log_Manager_Log(LOG_LEVEL_ERROR, module, format, ##__VA_ARGS__) 
#else
#define Log_Debug(module, format, ...) Log_Manager_Log(LOG_LEVEL_DEBUG, module, format, __VA_ARGS__)
#define Log_Info(module, format, ...) Log_Manager_Log(LOG_LEVEL_INFO, module, format, __VA_ARGS__)
#define Log_Warning(module, format, ...) Log_Manager_Log(LOG_LEVEL_WARNING, module, format, __VA_ARGS__)
#define Log_Error(module, format, ...) Log_Manager_Log(LOG_LEVEL_ERROR, module, format, __VA_ARGS__) 
#endif

#if defined(COMPILER_GCC)
#define Log_Debug_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_DEBUG, String_Lit(""), format, ##__VA_ARGS__)
#define Log_Info_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_INFO, String_Lit(""), format, ##__VA_ARGS__)
#define Log_Warning_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_WARNING, String_Lit(""), format, ##__VA_ARGS__)
#define Log_Error_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_ERROR, String_Lit(""), format, ##__VA_ARGS__) 
#else
#define Log_Debug_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_DEBUG, String_Lit(""), format, __VA_ARGS__)
#define Log_Info_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_INFO, String_Lit(""), format, __VA_ARGS__)
#define Log_Warning_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_WARNING, String_Lit(""), format, __VA_ARGS__)
#define Log_Error_Simple(format, ...) Log_Manager_Log(LOG_LEVEL_ERROR, String_Lit(""), format, __VA_ARGS__) 
#endif

#endif