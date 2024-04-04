string Date_To_String(allocator* Allocator, date Date) {
    scratch Scratch = Scratch_Get();
    string DayStr = String_To_Date_Format(&Scratch, Date.Day);
    return string(Allocator, "%d-%.*s-%.*s", Date.Year, MonthNumStr[(u32)Date.Month].Size, MonthNumStr[(u32)Date.Month].Str,
                  DayStr.Size, DayStr.Str);
}

string Time_To_String(allocator* Allocator, time Time) {
    scratch Scratch = Scratch_Get();
    string HourStr = String_To_Date_Format(&Scratch, Time.Hour);
    string MinuteStr = String_To_Date_Format(&Scratch, Time.Minute);
    string SecondStr = String_To_Date_Format(&Scratch, Time.Second);
    string MillisecondStr = String_To_Millisecond_Format(&Scratch, Time.Millisecond);

    return string(Allocator, "%.*s:%.*s:%.*s.%.*s", HourStr.Size, HourStr.Str, MinuteStr.Size, MinuteStr.Str, 
                  SecondStr.Size, SecondStr.Str, MillisecondStr.Size, MillisecondStr.Str);
}

#if defined(OS_WIN32)
date_time Date_Time_Now() {
    FILETIME FileTime;
    GetSystemTimePreciseAsFileTime(&FileTime);

    SYSTEMTIME SystemTime;
    FileTimeToSystemTime(&FileTime, &SystemTime);

    SYSTEMTIME LocalTime;
    SystemTimeToTzSpecificLocalTime(nullptr, &SystemTime, &LocalTime);

    return {
        .Date = {
            .Year      = LocalTime.wYear,
            .Month     = (month)(LocalTime.wMonth-1),
            .DayOfWeek = (day_of_week)(LocalTime.wDayOfWeek),
            .Day       = LocalTime.wDay
        },
        .Time = {
            .Hour        = LocalTime.wHour,
            .Minute      = LocalTime.wMinute,
            .Second      = LocalTime.wSecond,
            .Millisecond = LocalTime.wMilliseconds
        }
    };
}
#elif defined(OS_POSIX)
date_time Date_Time_Now() {
    timespec Timespec;
    clock_gettime(CLOCK_REALTIME, &Timespec);

    u32 Millisecond;
    if(Timespec.tv_nsec >= 999500000) {
        Timespec.tv_sec++;
        Millisecond = 0;
    } else {
        Millisecond = (u32)((Timespec.tv_nsec + 500000) / 1000000);
    }

    struct tm* Time = localtime(&Timespec.tv_sec);
    return {
        .Date = {
            .Year      = 1900+(u32)Time->tm_year,
            .Month     = (month)(Time->tm_mon+1),
            .DayOfWeek = (day_of_week)(Time->tm_wday),
            .Day       = (u32)Time->tm_mday
        },
        .Time = {
            .Hour        = (u32)Time->tm_hour,
            .Minute      = (u32)Time->tm_min,
            .Second      = (u32)Time->tm_sec,
            .Millisecond = Millisecond
        }
    };
}
#else
#error Not Implemented
#endif