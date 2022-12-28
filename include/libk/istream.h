#ifndef PINTOS_ISTREAM_H
#define PINTOS_ISTREAM_H

#include "ios.h"

namespace std_k {

template<typename CharT>
class basic_istream : virtual public basic_ios<CharT> {};

using istream = basic_istream<char>;

#endif // PINTOS_ISTREAM_H
