#ifndef DATETIME_H
#define DATETIME_H

enum class month : u32 {
    January,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December,
    Count
};

static const string MonthStr[] = {
    String_Lit("January"),
    String_Lit("February"),
    String_Lit("March"),
    String_Lit("April"),
    String_Lit("May"),
    String_Lit("June"),
    String_Lit("July"),
    String_Lit("August"),
    String_Lit("September"),
    String_Lit("October"),
    String_Lit("November"),
    String_Lit("December")
};

static const u32 MonthNum[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

static const string MonthNumStr[] = {
    String_Lit("01"), String_Lit("02"), String_Lit("03"), 
    String_Lit("04"), String_Lit("05"), String_Lit("06"), 
    String_Lit("07"), String_Lit("08"), String_Lit("09"), 
    String_Lit("10"), String_Lit("11"), String_Lit("12")
};

static_assert(Array_Count(MonthStr) == (u32)month::Count);
static_assert(Array_Count(MonthNum) == (u32)month::Count);
static_assert(Array_Count(MonthNumStr) == (u32)month::Count);

enum day_of_week : u32 {
    Sunday,
    Monday,
    Tuesday,
    Wednesday, 
    Thursday,
    Friday,
    Saturday,
    Count
};

static const string DayOfWeekStr[] = {
    String_Lit("Sunday"),
    String_Lit("Monday"),
    String_Lit("Tuesday"),
    String_Lit("Wednesday"), 
    String_Lit("Thursday"),
    String_Lit("Friday"),
    String_Lit("Saturday")
};

static_assert(Array_Count(DayOfWeekStr) == (u32)day_of_week::Count);

struct date {
    u32         Year;
    month       Month;
    day_of_week DayOfWeek;
    u32         Day;
};

string Date_To_String(allocator* Allocator, date Date);

struct time {
    u32 Hour;
    u32 Minute;
    u32 Second;
    u32 Millisecond;
};

string Time_To_String(allocator* Allocator, time Time);

struct date_time {
    date Date;
    time Time;
};

date_time Date_Time_Now();

#endif