#ifndef FASTREAD_VectorInput_H
#define FASTREAD_VectorInput_H

#include <boost/lexical_cast.hpp>

namespace fastread {

    template <typename Source>
    class VectorInput {
    public:

        VectorInput (Source* source_) {
            m_Source = source_;
        }

        virtual ~VectorInput() {}

        virtual SEXP get_as_string(CharacterVector na_strings = NA_STRING, bool trim = true) {
            SEXP val = m_Source->get_String(trim);

            bool is_na = false;
            for (unsigned int j = 0;j < na_strings.size();++j) {
                if (val == na_strings[j]) {
                    is_na = true;
                    break;
                }
            }

            return (is_na) ? NA_STRING : val;
        }

        virtual void set( int i, CharacterVector na_strings = NA_STRING ) = 0;
        virtual SEXP get() = 0;
        virtual bool skip() const { return false; }

    protected:
        Source* m_Source;
    };

    template <typename Source>
    class VectorInput_Logical : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;

        VectorInput_Logical(int n, Source* source_) : Base(source_) {
            m_Data = LogicalVector(no_init(n));
        }

        void set (int i, CharacterVector na_strings) {
            SEXP val = Base::get_as_string(na_strings);
            
            if (traits::is_na<STRSXP>(val)) {
                m_Data[i] = NA_LOGICAL;
                return;
            }
            
            std::string cur_val = as<std::string>(val);
            
            if (cur_val == "TRUE" || cur_val == "T")
                m_Data[i] = true;
            else if (cur_val == "FALSE" || cur_val == "F")
                m_Data[i] = false;
            else
                stop("Trying to parse non-logical value as logical type.");
        }

        inline SEXP get(){ return m_Data; }

    private:
        LogicalVector m_Data;
    };

    template <typename Source>
    class VectorInput_Integer : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;
        
        VectorInput_Integer (int n, Source* source_) : Base(source_) {
            m_Data = IntegerVector(no_init(n));
        }

        void set (int i, CharacterVector na_strings) {
            SEXP val = Base::get_as_string(na_strings);
            
            if (traits::is_na<STRSXP>(val)) {
                m_Data[i] = NA_INTEGER;
                return;
            }
            
            try {
                m_Data[i] = boost::lexical_cast<int>(as<std::string>(val));
            } catch( boost::bad_lexical_cast const& ) {
                stop("Trying to parse non-integer value as integer type.");
            }
        }
        
        inline SEXP get() { return m_Data; }

    private:
        IntegerVector m_Data;
    };

    template <typename Source>
    class VectorInput_Factor : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;
        typedef boost::unordered_map<SEXP,int> MAP;
        
        VectorInput_Factor (int n, Source* source_, CharacterVector levels_, bool ordered_) : Base(source_) {
            m_Data = IntegerVector(no_init(n));
            m_Levels = levels_;
            m_IsOrdered = ordered_;
            
            int m = m_Levels.size();
            
            // Train unordered map with int
            m_LevelMap = MAP();
            for (int i = 0;i < m;++i) {
                SEXP level = m_Levels[i];
                m_LevelMap[level] = i;
            }
        }
        
        void set (int i, CharacterVector na_strings) {
            SEXP st = Base::get_as_string(na_strings);

            if (traits::is_na<STRSXP>(st)) {
                m_Data[i] = NA_INTEGER;
                return;
            }

            MAP::iterator it = m_LevelMap.find(st) ;
            if (it == m_LevelMap.end()) {
                stop("Value not in list of allowed levels");
            } else {
                m_Data[i] = it->second + 1;
            }
        }

        SEXP get() {
            m_Data.attr("levels") = m_Levels;
            if (m_IsOrdered) {
                m_Data.attr("class") = CharacterVector::create("ordered", "factor");
            } else {
                m_Data.attr("class" ) = "factor";
            }

            return m_Data;
        }

    private:
        IntegerVector m_Data;
        CharacterVector m_Levels;
        bool m_IsOrdered;
        MAP m_LevelMap;
    };

    template <typename Source>
    class VectorInput_Double : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;
        
        VectorInput_Double (int n, Source* source_) : Base(source_) {
            m_Data = DoubleVector(no_init(n));
        }

        void set (int i, CharacterVector na_strings) {
            SEXP val = Base::get_as_string(na_strings);
            
            if (traits::is_na<STRSXP>(val)) {
                m_Data[i] = NA_REAL;
                return;
            }
            
            try {
                m_Data[i] = boost::lexical_cast<double>(as<std::string>(val));
            } catch( boost::bad_lexical_cast const& ) {
                stop("Trying to parse non-double value as double type.");
            }
        }
        
        inline SEXP get() { return m_Data; }

    private:
        DoubleVector m_Data;
    };

    template <typename Source>
    class VectorInput_String : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;
        
        VectorInput_String (int n, Source* source_, bool trim_) : Base(source_) {
            m_Data = CharacterVector(no_init(n));
            m_DoTrimming = trim_;
        }
        
        void set (int i, CharacterVector na_strings) {
            m_Data[i] = Base::get_as_string(na_strings, m_DoTrimming);
        }

        inline SEXP get() { return m_Data; }

    private:
        CharacterVector m_Data;
        bool m_DoTrimming;
    };

    template <typename Source>
    class VectorInput_Date_ymd : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;
        
        VectorInput_Date_ymd (int n, Source* source_) : Base(source_) {
            m_Data = NumericVector(no_init(n));
        }

        void set (int i, CharacterVector na_strings) {
            SEXP val = Base::get_as_string(na_strings);

            if (traits::is_na<STRSXP>(val)) {
                m_Data[i] = NA_REAL;
                return;
            }

            std::string cur_val = as<std::string>(val);
            char* start = const_cast<char*>(cur_val.c_str());
            char* end = start + cur_val.size();
            m_Data[i] = m_DateParser.parse_Date(start, end);
        }

        inline SEXP get() {
            m_Data.attr("class") = "Date";
            return m_Data;
        }

    private:
        NumericVector m_Data;
        DateTimeParser m_DateParser;
    };

    template <typename Source>
    class VectorInput_POSIXct : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;
        
        VectorInput_POSIXct (int n, Source* source_) : Base(source_) {
            m_Data = NumericVector(no_init(n));
        }

        void set (int i, CharacterVector na_strings) {
            SEXP val = Base::get_as_string(na_strings);

            if (traits::is_na<STRSXP>(val)) {
                m_Data[i] = NA_REAL;
                return;
            }

            std::string cur_val = as<std::string>(val);
            char* start = const_cast<char*>(cur_val.c_str());
            char* end = start + cur_val.size();
            m_Data[i] = m_POSIXctParser.parse_POSIXct(start, end);
        }

        inline SEXP get() {
            m_Data.attr("class") = CharacterVector::create( "POSIXct", "POSIXt" );
            return m_Data;
        }

    private:
        NumericVector m_Data;
        DateTimeParser m_POSIXctParser;
    };

    template <typename Source>
    class VectorInput_Time : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;
        
        VectorInput_Time (int n, Source* source_) : Base(source_) {
            m_Data = NumericVector(no_init(n));
        }

        void set (int i, CharacterVector na_strings) {
            SEXP val = Base::get_as_string(na_strings);

            if (traits::is_na<STRSXP>(val)) {
                m_Data[i] = NA_REAL;
                return;
            }

            std::string cur_val = as<std::string>(val);
            char* start = const_cast<char*>(cur_val.c_str());
            char* end = start + cur_val.size();
            m_Data[i] = m_TimeParser.parse_Time(start, end);
        }

        inline SEXP get() {
            m_Data.attr("class") = "Time";
            return m_Data;
        }

    private:
        NumericVector m_Data;
        DateTimeParser m_TimeParser;
    };


    template <typename Source>
    class VectorInput_Rownames : public VectorInput_String<Source> {
    public:
        typedef VectorInput_String<Source> Base;
        
        VectorInput_Rownames (int n, Source* source_) : Base(n, source_) {}
        
        bool is_rownames() const { return true; }
    };

    template <typename Source>
    class VectorInput_Skip : public VectorInput<Source> {
    public:
        typedef VectorInput<Source> Base;

        VectorInput_Skip (int, Source* source_) : Base(source_) {}
        
        void set (int i, CharacterVector na_strings) {
            Base::m_Source->skip() ;
        }
        
        inline SEXP get() { return R_NilValue; }
        virtual bool skip() const { return true; }
    } ;

    template <typename Source>
    VectorInput<Source>* create_parser(List spec, int n, Source& source) {
        String clazz = (as<CharacterVector>(spec["type"]))[0];

        if( clazz == "logical"   ) return new VectorInput_Logical<Source>(n, &source) ;
        if( clazz == "integer"   ) return new VectorInput_Integer<Source>(n, &source) ;
        if( clazz == "double"    ) return new VectorInput_Double<Source>(n, &source) ;
        if( clazz == "character" ) {
            bool trim = (as<LogicalVector>(spec["trim"]))[0];
            return new VectorInput_String<Source>(n, &source, trim) ;
        }
        if( clazz == "skip"      ) return new VectorInput_Skip<Source>(n, &source) ;
        if( clazz == "factor"    ) {
            CharacterVector levels = as<CharacterVector>(spec["levels"]);
            bool ordered = (as<LogicalVector>(spec["ordered"]))[0];
            return new VectorInput_Factor<Source>(n, &source, levels, ordered) ;
        }
        if( clazz == "Date"      ) return new VectorInput_Date_ymd<Source>(n, &source) ;
        if( clazz == "POSIXct"   ) return new VectorInput_POSIXct<Source>(n, &source) ;
        if( clazz == "Time"      ) return new VectorInput_Time<Source>(n, &source) ;
        stop( "unsupported column type" ) ;
        return 0 ;
    }

}

#endif
