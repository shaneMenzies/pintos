#ifndef PINTOS_IOS_H
#define PINTOS_IOS_H

#include "common.h"

namespace std_k {

template<typename CharT> class basic_streambuf;
template<typename CharT> class basic_ostream;

class ios_base {
  public:
    enum openmode {
        app    = 1 << 0,
        binary = 1 << 1,
        in     = 1 << 2,
        out    = 1 << 3,
        trunc  = 1 << 4,
        ate    = 1 << 5,
    };

    using fmtflags = uint16_t;
    enum {
        basefield = 0b11,
        dec       = 1,
        oct       = 2,
        hex       = 3,

        adjustfield = 0b11 << 2,
        left        = 1 << 2,
        right       = 2 << 2,
        internal    = 3 << 2,

        floatfield = 0b1 << 4,
        scientific = 1 << 4,
        fixed      = 0 << 4,

        boolalpha = 1 << 5,
        showbase  = 1 << 6,
        showpoint = 1 << 7,
        skipws    = 1 << 8,
        unitbuf   = 1 << 9,
        uppercase = 1 << 10,
    };
    fmtflags default_flags = dec | left | fixed;

    using iostate = uint8_t;
    enum {
        goodbit = 0,
        badbit  = 1 << 0,
        failbit = 1 << 1,
        eofbit  = 1 << 2,
    };
    iostate default_state = goodbit;

    enum seekdir { beg, end, cur };

    ios_base(const ios_base&) = delete;

    fmtflags flags() const { return base_flags; };
    fmtflags flags(fmtflags new_flags) {
        base_flags = new_flags;
        return base_flags;
    };

    unsigned int precision() const { return base_precision; };
    unsigned int precision(unsigned int new_precision) {
        base_precision = new_precision;
        return base_precision;
    };

    unsigned int width() const { return base_width; };
    unsigned int width(unsigned int new_width) {
        base_width = new_width;
        return base_width;
    };

  protected:
    fmtflags base_flags;

    unsigned int base_precision;
    unsigned int base_width;

    ios_base() {}
};

template<typename CharT> class basic_ios : public ios_base {
  public:
    explicit basic_ios(basic_streambuf<CharT>* sb) { init(sb); }
    virtual ~basic_ios() {}

    iostate rdstate() const { return base_state; }
    void    clear(iostate state = ios_base::goodbit) { base_state = state; }
    void    setstate(iostate state) { clear(base_state | state); }

    bool good() const { return (base_state == ios_base::goodbit); }
    bool eof() const { return (base_state & ios_base::eofbit); }
    bool fail() const { return (base_state & ios_base::failbit); }
    bool bad() const { return (base_state & ios_base::badbit); }

    bool     operator!() const { return (bad() || fail()); }
    explicit operator bool() const { return !fail(); }

    basic_ios& copyfmt(const basic_ios& other) {
        base_flags = other.base_flags;
        target     = other.target;
    }

    CharT fill() const { return fill_char; }
    CharT fill(CharT ch) {
        CharT old = fill_char;
        fill_char = ch;
        return old;
    }

    basic_streambuf<CharT>* rdbuf() const { return target; }
    basic_streambuf<CharT>* rdbuf(basic_streambuf<CharT>* new_buf) {
        basic_streambuf<CharT>* old = target;
        target                      = new_buf;
        clear();
        return old;
    }

    basic_ostream<CharT>* tie() const { return tied_stream; }
    basic_ostream<CharT>* tie(basic_ostream<CharT>* new_str) {
        basic_ostream<CharT>* old = tied_stream;
        tied_stream               = new_str;
        return old;
    }

  protected:
    void init(std_k::basic_streambuf<CharT>* new_target) {
        target    = new_target;
        fill_char = static_cast<CharT>(' ');

        base_flags = default_flags;
        base_state = default_state;
    }

  private:
    CharT                   fill_char;
    iostate                 base_state;
    basic_streambuf<CharT>* target      = nullptr;
    basic_ostream<CharT>*   tied_stream = nullptr;
};

using ios = basic_ios<char>;

} // namespace std_k

#endif // PINTOS_IOS_H
