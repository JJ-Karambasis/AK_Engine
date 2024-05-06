#ifndef PROFILER_H
#define PROFILER_H

#ifdef PROFILE_ENABLED
struct prof_stat {
    string Name;
    f64    AvgMS;
    u64    AvgHZ;
};

void            Prof_Start_Capture();
void            Prof_End_Capture();
void            Prof_Begin(const char* Format, ...);
void            Prof_End();
void            Prof_Tick();
span<prof_stat> Prof_Stats();


struct profile__scope {
    inline ~profile__scope() {
        Prof_End();
    }
};

#define Prof_Scope(format, ...) Prof_Begin(format, __VA_ARGS__); profile_scope _Scope_

#else
#define Prof_Start_Capture()
#define Prof_End_Capture()
#define Prof_Begin(...)
#define Prof_End()
#define Prof_Tick()
#define Prof_Scope(...)
#endif

#endif