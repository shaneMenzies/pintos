#ifndef PINTOS_OSTREAM_H
#define PINTOS_OSTREAM_H

#include "cstring.h"
#include "ios.h"
#include "streambuf.h"

namespace std_k {

template<typename CharT> class basic_ostream : virtual public basic_ios<CharT> {
  public:
    using char_type = CharT;

    // Sentry used to prepare and close all output operatotions
    class sentry {
      public:
        explicit sentry(basic_ostream<CharT>& target)
            : target(target) {
            if (!target.good()) return;

            // Flush tied stream first
            basic_ostream<CharT>* tied = target.tie();
            if (tied != nullptr) { tied->flush(); }

            valid = target.good();
        }

        ~sentry() {
            // Check if we need to flush after every operation
            if ((target.flags() & ios_base::unitbuf) && target.good()) {
                if (target.rdbuf()->pubsync() == -1) {
                    // Flush failed
                    target.setstate(ios_base::badbit);
                }
            }
        }

        explicit operator bool() const { return valid; }

      private:
        basic_ostream<CharT>& target;
        bool                  valid = false;
    };

    explicit basic_ostream(basic_streambuf<CharT>* sb)
        : basic_ios<CharT>(sb) {}
    virtual ~basic_ostream() {}

    basic_ostream<CharT>& put(char_type ch) {
        sentry valid(*this);
        if (valid) { basic_ios<CharT>::rdbuf()->sputc(ch); }
        ~valid();
        return *this;
    }

    basic_ostream<CharT>& write(const char_type* source, size_t count) {
        sentry valid(*this);
        if (valid) { basic_ios<CharT>::rdbuf()->sputn(source, count); }
        ~valid();
        return *this;
    }

    basic_ostream<CharT>& flush() {
        sentry valid(*this);
        if (valid) { basic_ios<CharT>::rdbuf()->pubsync(); }
        ~valid();
        return *this;
    }

    size_t tellp() {
        sentry valid(*this);
        size_t result = ~0;
        if (valid) {
            result = basic_ios<CharT>::rdbuf()->pubseekoff(0, ios_base::cur,
                                                           ios_base::out);
        }
        ~valid();
        return result;
    }
    basic_ostream& seekp(size_t pos) {
        sentry valid(*this);
        if (valid) {
            basic_ios<CharT>::rdbuf()->pubseekpos(pos, ios_base::out);
        }
        ~valid();
        return *this;
    }
    basic_ostream& seekp(int off, ios_base::seekdir dir) {
        sentry valid(*this);
        if (valid) {
            basic_ios<CharT>::rdbuf()->pubseekoff(off, dir, ios_base::out);
        }
        ~valid();
        return *this;
    }

    // region << Operations
    basic_ostream& operator<<(CharT value) { return put(value); }
    basic_ostream& operator<<(const CharT* value) {
        sentry valid(*this);
        if (!valid) return *this;

        size_t i = 0;
        while (value[i] != (CharT)'\0') {
            basic_ios<CharT>::rdbuf()->sputc(value[i]);
        }

        ~valid();
        return *this;
    }

    basic_ostream& operator<<(short value) { return operator<<((long)value); }
    basic_ostream& operator<<(unsigned short value) {
        return operator<<((unsigned long)value);
    }

    basic_ostream& operator<<(int value) { return operator<<((long)value); }
    basic_ostream& operator<<(unsigned int value) {
        return operator<<((unsigned long)value);
    }

    basic_ostream& operator<<(long value) {
        // Oct and Hex Bases always shown as unsigned
        if ((ios_base::flags() & ios_base::basefield) != ios_base::dec) {
            return operator<<((unsigned long)value);
        }

        sentry valid(*this);
        if (!valid) return *this;

        // Base can only be decimal at this point
        size_t base = 10;

        // Signed 64-bit conversion
        char   buffer[32];
        size_t i        = 0;
        bool   negative = (value < 0);
        if (negative) value = -value;

        while (value) {
            // Convert to an ascii character
            char digit = (value % base) + '0';
            if (digit > '9') {
                // Digits past 9 move to letters
                digit += ('A' - '9');
            }

            // Place and increment
            buffer[i] = digit;
            i++;
            value /= base;
        }

        // Add - if number was negative
        if (negative) {
            buffer[i] = '-';
            i++;
        }

        // Can print to buffer
        basic_ios<CharT>::rdbuf()->sputn(&buffer[i - 1], i);

        ~valid();
        return *this;
    }
    basic_ostream& operator<<(unsigned long value) {
        sentry valid(*this);
        if (!valid) return *this;

        // Get base from flags
        size_t base;
        switch (ios_base::flags() & ios_base::basefield) {
            case ios_base::dec: base = 10; break;
            case ios_base::oct: base = 8; break;
            case ios_base::hex: base = 16; break;
        }

        // Unsigned 64-bit conversion
        char   buffer[32];
        size_t i = 0;
        while (value) {
            // Convert to an ascii character
            char digit = (value % base) + '0';
            if (digit > '9') {
                // Digits past 9 move to letters
                digit += ('A' - '9');
            }

            // Place and increment
            buffer[i] = digit;
            i++;
            value /= base;
        }

        // Check if base prefix is needed
        if (ios_base::flags() & ios_base::showbase) {
            switch (base) {
                case 8:
                    // Octal prefix: 0
                    buffer[i] = '0';
                    i++;
                    break;

                case 16:
                    // Hexadecimal prefix: 0x
                    buffer[i]     = 'x';
                    buffer[i + 1] = '0';
                    i += 2;
                    break;
            }
        }

        // Can print to buffer
        basic_ios<CharT>::rdbuf()->sputn(&buffer[i - 1], i);

        ~valid();
        return *this;
    }

    basic_ostream& operator<<(float value) {
        return operator<<(static_cast<long double>(value));
    }
    basic_ostream& operator<<(double value) {
        return operator<<(static_cast<long double>(value));
    }
    basic_ostream& operator<<(long double value) {
        sentry valid(*this);
        if (!valid) return *this;

        // Split between scientific and fixed notation
        if ((ios_base::flags() & ios_base::floatfield)
            == ios_base::scientific) {

            // Seperate into power and fraction
            int         power;
            long double fraction = __builtin_frexpl(value, &power);

            size_t base     = 10;
            bool   negative = (fraction < 0);
            if (negative) { fraction = -fraction; }

            // Print fraction
            const size_t max_i = sizeof(long double) * 8;
            char         buffer[max_i + 8];
            buffer[0] = '\0';
            size_t i  = 1;

            // If negative add -
            if (negative) {
                buffer[i] = '-';
                i++;
            }

            // Start with first digit and decimal point
            buffer[i] = (char)__builtin_truncl(fraction) + '0';
            fraction -= __builtin_truncl(fraction);
            buffer[i + 1] = '.';
            i += 2;
            fraction *= 10;

            // Fill rest of buffer backwards
            while ((fraction != 0.0) && (i < max_i)) {
                char digit = (char)__builtin_truncl(fraction);
                fraction -= digit;
                buffer[i] = (digit + '0');
                i++;
                fraction *= 10;
            }

            // Add power prefix
            buffer[i]     = 'e';
            buffer[i + 1] = '+';
            i += 2;

            // Reverse the buffer
            std_k::strrev(buffer, &buffer[i - 1]);

            // Can print fraction part (excludes null terminator)
            basic_ios<CharT>::rdbuf()->sputn(&buffer[i - 1], i - 1);

            // Print exponent as an integer
            operator<<(power);

        } else {

            // Seperate into integer and fraction
            long double integer_ld;
            long double fraction = __builtin_modfl(value, &integer_ld);

            // Put - sign on integer part, not fraction
            if (fraction < 0) {
                fraction = -fraction;
                if (integer_ld >= 0) { integer_ld = -integer_ld; }
            }

            // Start with printing integer
            operator<<(__builtin_truncl(integer_ld));

            // Print fraction
            size_t       base  = 10;
            const size_t max_i = sizeof(long double) * 8;
            char         buffer[max_i + 8];
            buffer[0] = '\0';
            size_t i  = 1;

            // Start with decimal point
            buffer[i] = '.';
            i++;
            fraction *= 10;

            // Fill rest of buffer backwards
            while ((fraction != 0.0) && (i < max_i)) {
                char digit = (char)__builtin_truncl(fraction);
                fraction -= digit;
                buffer[i] = (digit + '0');
                i++;
                fraction *= 10;
            }

            // Reverse the buffer
            std_k::strrev(buffer, &buffer[i - 1]);

            // Can print fraction part (excludes null terminator)
            basic_ios<CharT>::rdbuf()->sputn(&buffer[i - 1], i - 1);
        }
        ~valid();
        return *this;
    }

    basic_ostream& operator<<(bool value) {
        sentry valid(*this);
        if (!valid) return *this;

        if (ios_base::flags() & ios_base::boolalpha) {
            // Alphabetic representation
            if (value) {
                basic_ios<CharT>::rdbuf()->sputn("true", 4);
            } else {
                basic_ios<CharT>::rdbuf()->sputn("false", 5);
            }
        } else {
            // Binary representation
            if (value) {
                basic_ios<CharT>::rdbuf()->sputc('1');
            } else {
                basic_ios<CharT>::rdbuf()->sputc('0');
            }
        }

        ~valid();
        return *this;
    }

    basic_ostream& operator<<(const void* value) {
        sentry valid(*this);
        if (!valid) return *this;

        // Pointer always gets printed in hex, as an unsigned long
        size_t base = 16;
        char   buffer[32];
        size_t i = 0;
        while ((uintptr_t)value) {
            // Convert to an ascii character
            char digit = ((uintptr_t)value % base) + '0';
            if (digit > '9') {
                // Digits past 9 move to letters
                digit += ('A' - '9');
            }

            // Place and increment
            buffer[i] = digit;
            i++;
            value = (const void*)((uintptr_t)value / base);
        }

        // Add base prefix
        buffer[i]     = 'x';
        buffer[i + 1] = '0';
        i += 2;

        // Can print to buffer
        basic_ios<CharT>::rdbuf()->sputn(&buffer[i - 1], i);

        ~valid();
        return *this;
    }
    // endregion
};

using ostream = basic_ostream<char>;

} // namespace std_k

#endif // PINTOS_OSTREAM_H
