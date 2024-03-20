string Log_Event_To_String(allocator* Allocator, log_event* LogEvent) {
    scratch Scratch = Scratch_Get();
    string Date = Date_To_String(&Scratch, LogEvent->DateTime.Date);
    string Time = Time_To_String(&Scratch, LogEvent->DateTime.Time);
    return string(Allocator, "%.*s %.*s - %.*s - %.*s - %.*s", Date.Size, Date.Str, Time.Size, Time.Str, 
                  LogEvent->Module.Size, LogEvent->Module.Str, G_LogLevelStr[(u32)LogEvent->Level].Size, G_LogLevelStr[(u32)LogEvent->Level].Str, 
                  LogEvent->Message.Size, LogEvent->Message.Str);
}

internal void Logger_Pool__Init(logger_pool* LoggerPool) {
    Zero_Struct(LoggerPool);
    LoggerPool->FirstFreeIndex = (u32)-1;
    
    for(u32 i = 0; i < MAX_THREAD_COUNT; i++) {
        LoggerPool->Loggers[i].PoolIndex = (u32)-1;
        LoggerPool->Loggers[i].PoolNextIndex = (u32)-1;
    }
}

internal logger* Logger_Pool__Allocate_Logger(logger_pool* Pool) {
    u32 Index;
    if(Pool->FirstFreeIndex != (u32)-1) {
        Index = Pool->FirstFreeIndex;
        Pool->FirstFreeIndex = Pool->Loggers[Index].PoolNextIndex;
    } else {
        Assert(Pool->MaxUsed < MAX_THREAD_COUNT);
        Index = Pool->MaxUsed++;
    }

    logger* Logger = Pool->Loggers + Index;
    Logger->PoolNextIndex = (u32)-1;
    Logger->PoolIndex = Index;
    
    return Logger;
}

internal void Logger_Pool__Delete_Logger(logger_pool* Pool, logger* Logger) {
    Logger->PoolNextIndex = Pool->FirstFreeIndex;
    Pool->FirstFreeIndex = Logger->PoolIndex;
    Logger->PoolIndex = (u32)-1;
}

internal void Logger_Map__Add(logger_map* LoggerMap, u64 ThreadID, logger* Logger) {
    u32 Hash = Hash_U64(ThreadID);
    u32 SlotIndex = Hash % MAX_THREAD_COUNT;
    logger_slot* Slot = LoggerMap->Slots + SlotIndex;
    DLL_Push_Back_NP(Slot->SlotHead, Slot->SlotTail, Logger, HashNext, HashPrev);
}

internal void Logger_Map__Remove(logger_map* LoggerMap, u64 ThreadID) {
    u32 Hash = Hash_U64(ThreadID);
    u32 SlotIndex = Hash % MAX_THREAD_COUNT;
    logger_slot* Slot = LoggerMap->Slots + SlotIndex;

    logger* TargetLogger = NULL;
    for(logger* Logger = Slot->SlotHead; Logger; Logger = Logger->HashNext) {
        if(Logger->ThreadID == ThreadID) {
            TargetLogger = Logger;
            break;
        }
    }

    if(TargetLogger) {
        DLL_Remove_NP(Slot->SlotHead, Slot->SlotTail, TargetLogger, HashNext, HashPrev);
    }
}

internal logger* Logger_Map__Get(logger_map* LoggerMap, u64 ThreadID) {
    u32 Hash = Hash_U64(ThreadID);
    u32 SlotIndex = Hash % MAX_THREAD_COUNT;
    logger_slot* Slot = LoggerMap->Slots + SlotIndex; 

    for(logger* Logger = Slot->SlotHead; Logger; Logger = Logger->HashNext) {
        if(Logger->ThreadID == ThreadID) {
            return Logger;
        }
    }

    return NULL;
}

logger* Log_Manager__Create_Logger(log_manager* LogManager, u64 ThreadID) {
    AK_Mutex_Lock(&LogManager->AllocateLock);
    logger* Result = Logger_Pool__Allocate_Logger(&LogManager->LoggerPool);
    AK_Mutex_Unlock(&LogManager->AllocateLock);

    Result->ThreadID = ThreadID;

    AK_Mutex_Lock(&LogManager->MapLock);
    Logger_Map__Add(&LogManager->LoggerMap, Result->ThreadID, Result);
    AK_Mutex_Unlock(&LogManager->MapLock);

    return Result;
}

logger* Log_Manager__Get_Logger() {
    log_manager* LogManager = Log_Manager_Get();
    if(LogManager) {
        logger* Logger = (logger*)AK_TLS_Get(&LogManager->LoggerLocalStorage);
        if(!Logger) {
            u64 ThreadID = AK_Thread_Get_Current_ID();

            AK_Mutex_Lock(&LogManager->MapLock);
            Logger = Logger_Map__Get(&LogManager->LoggerMap, ThreadID);
            AK_Mutex_Unlock(&LogManager->MapLock);

            if(!Logger) {
                Logger = Log_Manager__Create_Logger(LogManager, ThreadID);
            }

            AK_TLS_Set(&LogManager->LoggerLocalStorage, Logger);
        }

        return Logger;
    }
    return NULL;
}

void Logger__Log(logger* Logger, log_level Level, string Module, const char* Format, va_list List) {
    scratch Scratch = Scratch_Get();
    
    log_event LogEvent = {
        .DateTime = Date_Time_Now(),
        .Module = Module,
        .Level = Level,
        .Message = string(&Scratch, Format, List)
    };

#ifdef DEBUG_BUILD
    string  DebugLog = Log_Event_To_String(&Scratch, &LogEvent);
    
#if defined(OS_WIN32)
    wstring DebugLogW(&Scratch, DebugLog);
    OutputDebugStringW(DebugLogW.Str);
    OutputDebugStringA("\n");
#endif

#endif
}

logger* Logger_Get() {
    return Log_Manager__Get_Logger();
}

void Logger_Delete(logger* Logger) {
    log_manager* LogManager = Log_Manager_Get();
    if(LogManager && Logger) {
        AK_Mutex_Lock(&LogManager->MapLock);
        Logger_Map__Remove(&LogManager->LoggerMap, Logger->ThreadID);
        AK_Mutex_Unlock(&LogManager->MapLock);

        AK_Mutex_Lock(&LogManager->AllocateLock);
        Logger_Pool__Delete_Logger(&LogManager->LoggerPool, Logger);
        AK_Mutex_Unlock(&LogManager->AllocateLock);
    }
}

log_manager* Log_Manager_Create() {
    allocator* Allocator = Core_Get_Base_Allocator();
    log_manager* LogManager = Allocator_Allocate_Struct(Allocator, log_manager);
    Zero_Struct(LogManager);

    AK_Mutex_Create(&LogManager->AllocateLock);
    AK_Mutex_Create(&LogManager->MapLock);
    Logger_Pool__Init(&LogManager->LoggerPool);
    AK_TLS_Create(&LogManager->LoggerLocalStorage);

    Log_Manager_Set(LogManager);
    return LogManager;
}

void Log_Manager_Delete() {
    log_manager* LogManager = Log_Manager_Get();
    if(LogManager) {
        AK_TLS_Delete(&LogManager->LoggerLocalStorage);

        for(u32 LoggerIndex = 0; LoggerIndex < MAX_THREAD_COUNT; LoggerIndex++) {
            logger* Logger = LogManager->LoggerPool.Loggers + LoggerIndex;
            if(Logger->PoolIndex != (u32)-1) {
                Logger_Delete(Logger);
            }
        }

        AK_Mutex_Delete(&LogManager->AllocateLock);
        AK_Mutex_Delete(&LogManager->MapLock);

        allocator* Allocator = Core_Get_Base_Allocator();
        Allocator_Free_Memory(Allocator, LogManager);
        Log_Manager_Set(NULL);
    }
}

void Log_Manager_Log(log_level Level, string Module, const char* Format, ...) {
    logger* Logger = Logger_Get();
    va_list List;
    va_start(List, Format);
    Logger__Log(Logger, Level, Module, Format, List);
    va_end(List);
}

global log_manager* G_LogManager;
log_manager* Log_Manager_Get() {
    Assert(G_LogManager);
    return G_LogManager;
}

void Log_Manager_Set(log_manager* Manager) {
    G_LogManager = Manager;
}