#ifndef FASTREAD_DATE_TIME_PARSER_H
#define FASTREAD_DATE_TIME_PARSER_H

namespace fastread {

    class DateTimeParser {
    public:

        DateTimeParser () {
            m_DaysAtMonthStart.resize(m_MonthsInYear);
            int d = 0;
            m_DaysAtMonthStart[0] = d;
            d += 31;
            m_DaysAtMonthStart[1] = d;
            d += 28;
            m_DaysAtMonthStart[2] = d;
            d += 31;
            m_DaysAtMonthStart[3] = d;
            d += 30;
            m_DaysAtMonthStart[4] = d;
            d += 31;
            m_DaysAtMonthStart[5] = d;
            d += 30;
            m_DaysAtMonthStart[6] = d;
            d += 31;
            m_DaysAtMonthStart[7] = d;
            d += 31;
            m_DaysAtMonthStart[8] = d;
            d += 30;
            m_DaysAtMonthStart[9] = d;
            d += 31;
            m_DaysAtMonthStart[10] = d;
            d += 30;
            m_DaysAtMonthStart[11] = d;
        }

        /**
         * The number of days since the epoc
         */
        double parse_Date(char* start, char* end_) {
            p = start;
            end = end_;

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-date value as date type.");

            double res = 0;

            // year
            int cur_year = read_year();
            res += (cur_year - m_BaselineYear) * m_DaysInYear;

            // leap years between cur_year and m_BaselineYear
            res += (count_leap_years(cur_year) - count_leap_years(m_BaselineYear));

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-date value as date type.");

            // month
            int m = read_month();
            res += days_at_month_start(m);

            bool is_leap = is_leap_year(cur_year);
            if (cur_year > m_BaselineYear && m > 2)
                res += is_leap;
            else if (cur_year < m_BaselineYear && m < 3)
                res -= is_leap;

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-date value as date type.");

            // day
            int d = read_day();
            if (d == 29 && m == 2 && !is_leap)
                stop("Cannot parse a Feb, 29th of non-leap year as date.");
            res += d - 1;

            return res;
        }

        double parse_Time(char* start, char* end_) {
            p = start;
            end = end_;

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-time value as time type.");

            double res = 0;

            // hour
            int h = read_hour();
            res += h * m_SecondsInHour;

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-time value as time type.");

            // minutes
            int m = read_minute();
            res += m * m_SecondsInMinute;

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-time value as time type.");

            // seconds
            int s = read_second();
            res += s;

            return res;
        }


        /**
         * The number of seconds since the epoc
         */
        double parse_POSIXct(char* start, char* end_) {
            p = start;
            end = end_;

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-date value as DateTime type.");

            double res = 0;

            // year
            int cur_year = read_year();
            res += (cur_year - m_BaselineYear) * m_DaysInYear;

            // leap years between cur_year and m_BaselineYear
            res += (count_leap_years(cur_year) - count_leap_years(m_BaselineYear));

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-date value as DateTime type.");

            // month
            int m = read_month();
            res += days_at_month_start(m);

            bool is_leap = is_leap_year(cur_year);
            if (cur_year > m_BaselineYear && m > 2)
                res += is_leap;
            else if (cur_year < m_BaselineYear && m < 3)
                res -= is_leap;

            skip_non_digit();
            if (!has_more())
                stop("Trying to parse non-date value as DateTime type.");

            // day
            int d = read_day();
            if (d == 29 && m == 2 && !is_leap)
                stop("Cannot parse a Feb, 29th of non-leap year as date.");
            res += d - 1;

            res *= m_SecondsInDay;

            skip_non_digit() ;
            if (!has_more())
                stop("Trying to parse a DateTime without time.");

            res += parse_Time(p, end);

            return res;
        }

    private:
        char* p;
        char* end;

        static const int m_BaselineYear = 1970;
        static const int m_MonthsInYear = 12;
        static const int m_DaysInYear = 365;
        static const int m_SecondsInDay = 86400;
        static const int m_SecondsInHour = 3600;
        static const int m_SecondsInMinute = 60;
        
        std::vector<int> m_DaysAtMonthStart;

        inline int days_at_month_start(int m) {
            return m_DaysAtMonthStart[m-1];
        }

        inline bool valid_digit() {
            return *p >= '0' && *p <= '9';
        }

        inline int digit_value() {
            return *p - '0' ;
        }

        inline int read_year() {
            int y = read_int();
            if (y < 0)
                stop("Cannot parse date with a negative year.");
            return y;
        }

        inline int read_month() {
            int m = read_int();
            if (m < 1 || m > 12)
                stop("Cannot parse date with month not in [1,12].");
            return m;
        }

        inline int read_day() {
            int d = read_int();
            if (d < 1 || d > 31)
                stop("Cannot parse date with day not in [1,31].");
            return d;
        }

        inline int read_hour() {
            int h = read_int();
            if (h < 0 || h > 23)
                stop("Cannot parse time with hour not in [1,23].");
            return h;
        }

        inline int read_minute() {
            int m = read_int();
            if (m < 0 || m > 59)
                stop("Cannot parse time with minute not in [0,59].");
            return m;
        }

        inline int read_second() {
            int s = read_int();
            if (s < 0 || s > 59)
                stop("Cannot parse time with second not in [0,59].");
            return s;
        }

        inline int read_int() {
            int m = 0;
            while (valid_digit() && has_more()) {
                m = m*10 + digit_value();
                p++;
            }
            return m;
        }

        inline void skip_non_digit() {
            while (!valid_digit() && has_more())
                p++;
        }

        inline bool has_more() {
            return p != end;
        }

        inline int count_leap_years(int cur_year) {
            if (cur_year == 0)
                return 0;
            int res = 0;
            int direction = (cur_year > m_BaselineYear);
            res += (cur_year - direction) /   4;
            res -= (cur_year - direction) / 100;
            res += (cur_year - direction) / 400;
            return res;
        }

        inline bool is_leap_year(int cur_year) {
            bool res;
            if (cur_year%4 != 0)
                res = false;
            else if (cur_year%100 != 0)
                res = true;
            else if (cur_year%400 != 0)
                res = false;
            else
                res = true;
            return res;
        }

    };

}

#endif
