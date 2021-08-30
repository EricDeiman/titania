#ifndef MREDUTIL_HH
#define MREDUTIL_HH

// From:
// https://stackoverflow.com/questions/760301/implementing-a-no-op-stdostream/760353#760353

#include <streambuf>
#include <ostream>

template <class cT, class traits = std::char_traits<cT> >
class basic_nullbuf: public std::basic_streambuf<cT, traits> {
    typename traits::int_type overflow(typename traits::int_type c)
    {
        return traits::not_eof(c); // indicate success
    }
};

template <class cT, class traits = std::char_traits<cT> >
class basic_onullstream: public std::basic_ostream<cT, traits> {
    public:
        basic_onullstream():
        std::basic_ios<cT, traits>(&m_sbuf),
        std::basic_ostream<cT, traits>(&m_sbuf)
        {
            init(&m_sbuf);
        }

    private:
        basic_nullbuf<cT, traits> m_sbuf;
};

using nullstream = basic_onullstream<char>;
using wnullstream = basic_onullstream<wchar_t>;

#endif