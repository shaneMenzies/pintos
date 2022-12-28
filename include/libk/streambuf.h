#ifndef PINTOS_STREAMBUF_H
#define PINTOS_STREAMBUF_H

#include "common.h"
#include "ios.h"

namespace std_k {

template<typename CharT> class basic_streambuf {
  public:
    using char_type = CharT;

    virtual ~basic_streambuf() {}

    basic_streambuf<CharT>* pubsetbuf(char_type* s, size_t n) {
        return setbuf(s, n);
    }

    size_t pubseekoff(int off, ios_base::seekdir dir, ios_base::openmode side) {
        return seekoff(off, dir, side);
    }
    size_t pubseekpos(int pos, ios_base::openmode side) {
        return seekpos(pos, side);
    }
    int pubsync() { return sync(); }

    // region Output / Get functions

    size_t in_avail() { return (out_current - out_start); }

    int sbumbc() {
        int result = *out_current;
        out_current--;
        if (out_current < out_start) { underflow(); }

        return result;
    }
    int sgetc() {
        if (out_current < out_start) {
            return underflow();
        } else {
            return *out_current;
        }
    }
    int snextc() {
        if (out_current < out_start) { underflow(); }
        out_current--;
        return sgetc();
    }

    size_t sgetn(char_type* target, size_t count) {
        return xsgetn(target, count);
    }

    int sputbackc(char_type value) {
        if (out_current < out_end && out_current[1] == value) {
            out_current++;
            return 0;
        } else {
            return pbackfail(static_cast<int>(value));
        }
    }
    int sungetc() {
        if (out_current < out_end) {
            out_current++;
            return 0;
        } else {
            return pbackfail(-1);
        }
    }

    // endregion

    // region Input / Put functions

    int sputc(char_type value) {
        if (in_current < in_end) {
            *in_current = value;
            in_current++;
            return 0;
        } else {
            return overflow(value);
        }
    }
    size_t sputn(const char_type* source, size_t count) {
        return xsputn(source, count);
    }

    // endregion

  protected:
    basic_streambuf()
        : out_start(nullptr)
        , out_current(nullptr)
        , out_end(nullptr)
        , in_start(nullptr)
        , in_current(nullptr)
        , in_end(nullptr) {}

    basic_streambuf(const basic_streambuf<CharT>& rhs)
        : out_start(rhs.out_start)
        , out_current(rhs.out_current)
        , out_end(rhs.out_end)
        , in_start(rhs.in_start)
        , in_current(rhs.in_current)
        , in_end(rhs.in_end) {}

    virtual basic_streambuf<CharT>* setbuf(char_type* s, size_t n) {
        sync();

        in_start   = s;
        in_end     = in_start + (n / 2);
        in_current = in_start;

        out_start   = in_end;
        out_end     = in_start + n;
        out_current = out_start;

        return this;
    }

    virtual size_t seekoff(int off, ios_base::seekdir dir,
                           ios_base::openmode side) {
        size_t result = 0;
        if (side & ios_base::openmode::in) {
            switch (dir) {
                case ios_base::seekdir::beg: in_current = in_start + off; break;

                case ios_base::seekdir::end: in_current = in_end + off; break;

                default:
                case ios_base::seekdir::cur:
                    in_current = in_current + off;
                    break;
            }

            result = (in_current - in_start);
        }

        if (side & ios_base::openmode::out) {
            switch (dir) {
                case ios_base::seekdir::beg:
                    out_current = out_start + off;
                    break;

                case ios_base::seekdir::end: out_current = out_end + off; break;

                default:
                case ios_base::seekdir::cur:
                    out_current = out_current + off;
                    break;
            }

            result = (out_current - out_start);
        }

        return result;
    }

    virtual size_t seekpos(size_t pos, ios_base::openmode side) {
        if (side & ios_base::openmode::in) { in_current = in_start + pos; }

        if (side & ios_base::openmode::out) { out_current = out_start + pos; }

        return pos;
    }

    virtual int sync() { return 0; }

    // region Output / Get functions

    virtual size_t showmanyc() { return 0; }

    virtual int underflow() { return 0; }
    virtual int uflow() {
        int result = underflow();
        out_current--;
        return result;
    }

    virtual size_t xsgetn(char_type* target, size_t count) {
        size_t i = 0;
        while (i < count) {
            if (out_current < out_start) { underflow(); }

            target[i] = *out_current;
            out_current--;
            i++;
        }

        return i;
    }

    char_type* eback() const { return out_start; }
    char_type* gptr() const { return out_current; }
    char_type* egptr() const { return out_end; }
    void       setg(char_type* gbeg, char_type* gcurr, char_type* gend) {
        out_start   = gbeg;
        out_current = gcurr;
        out_end     = gend;
    }

    void gbump(int count) { out_current += count; }

    virtual int pbackfail(int value) {
        (void)value;
        return -1;
    }

    // endregion

    // region Input / Put Functions

    virtual int overflow(char_type value) {
        (void)value;
        return 0;
    }

    virtual size_t xsputn(const char_type* source, size_t count) {
        size_t i = 0;
        while (i < count) {
            if (in_current >= in_end) {
                overflow(source[i]);
            } else {
                *in_current = source[i];
                in_current++;
            }
            i++;
        }

        return i;
    }

    char_type* pbase() const { return in_start; }
    char_type* pptr() const { return in_current; }
    char_type* epptr() const { return in_end; }
    void       setp(char_type* pbeg, char_type* pend) {
        in_start   = pbeg;
        in_current = pbeg;
        in_end     = pend;
    }

    void pbump(int count) { in_current += count; }

    // endregion

  private:
    // Output of this stream, AKA get area
    CharT* out_start;
    CharT* out_current;
    CharT* out_end;

    // Input to this stream, AKA put area
    CharT* in_start;
    CharT* in_current;
    CharT* in_end;
};

using streambuf = basic_streambuf<char>;

} // namespace std_k

#endif // PINTOS_STREAMBUF_H
