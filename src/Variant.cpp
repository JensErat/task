////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2015, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <Variant.h>
#include <ISO8601.h>
#include <Lexer.h>
#include <Date.h>
#include <Duration.h>
#include <RX.h>
#include <text.h>
#include <i18n.h>

std::string Variant::dateFormat = "";
bool Variant::searchCaseSensitive = true;
bool Variant::searchUsingRegex = true;
bool Variant::isoEnabled = true;

////////////////////////////////////////////////////////////////////////////////
Variant::Variant ()
: _type (type_unknown)
, _bool (false)
, _integer (0)
, _real (0.0)
, _string ("")
, _date (0)
, _duration (0)
, _source ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const Variant& other)
: _type (other._type)
, _bool (other._bool)
, _integer (other._integer)
, _real (other._real)
, _string (other._string)
, _date (other._date)
, _duration (other._duration)
, _source (other._source)
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const bool value)
: _type (Variant::type_boolean)
, _bool (value)
, _integer (0)
, _real (0.0)
, _string ("")
, _date (0)
, _duration (0)
, _source ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const int value)
: _type (Variant::type_integer)
, _bool (false)
, _integer (value)
, _real (0.0)
, _string ("")
, _date (0)
, _duration (0)
, _source ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const double value)
: _type (Variant::type_real)
, _bool (false)
, _integer (0)
, _real (value)
, _string ("")
, _date (0)
, _duration (0)
, _source ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const std::string& value)
: _type (Variant::type_string)
, _bool (false)
, _integer (0)
, _real (0.0)
, _string (value)
, _date (0)
, _duration (0)
, _source ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const char* value)
: _type (Variant::type_string)
, _bool (false)
, _integer (0)
, _real (0.0)
, _string (std::string (value))
, _date (0)
, _duration (0)
, _source ("")
{
}

////////////////////////////////////////////////////////////////////////////////
Variant::Variant (const time_t value, const enum type new_type /*=type_date*/)
: _type (new_type)
, _bool (false)
, _integer (0)
, _real (0.0)
, _string ("")
, _date (0)
, _duration (0)
, _source ("")
{
  switch (new_type)
  {
  case type_date:     _date = value;     break;
  case type_duration: _duration = value; break;
  default:
    throw std::string (STRING_VARIANT_TIME_T);
  }
}

////////////////////////////////////////////////////////////////////////////////
Variant::~Variant ()
{
}

////////////////////////////////////////////////////////////////////////////////
void Variant::source (const std::string& input)
{
  _source = input;
}

////////////////////////////////////////////////////////////////////////////////
std::string Variant::source () const
{
  return _source;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator= (const Variant& other)
{
  if (this != &other)
  {
    _type     = other._type;
    _bool     = other._bool;
    _integer  = other._integer;
    _real     = other._real;
    _string   = other._string;
    _date     = other._date;
    _duration = other._duration;
    _source   = other._source;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator&& (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_boolean);
  right.cast (type_boolean);

  return left._bool && right._bool;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator|| (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_boolean);
  right.cast (type_boolean);

  return left._bool || right._bool;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_xor (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_boolean);
  right.cast (type_boolean);

  return (left._bool && !right._bool) ||
         (!left._bool && right._bool);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator< (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:                             return !left._bool && right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  < right._integer;
    case type_real:     left.cast (type_real);     return left._real     < right._real;
    case type_string:   left.cast (type_string);   return left._string   < right._string;
    case type_date:     left.cast (type_date);     return left._date     < right._date;
    case type_duration: left.cast (type_duration); return left._duration < right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_integer); return left._integer  < right._integer;
    case type_integer:                             return left._integer  < right._integer;
    case type_real:     left.cast (type_real);     return left._real     < right._real;
    case type_string:   left.cast (type_string);   return left._string   < right._string;
    case type_date:     left.cast (type_date);     return left._date     < right._date;
    case type_duration: left.cast (type_duration); return left._duration < right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_real);    return left._real     < right._real;
    case type_integer:  right.cast (type_real);    return left._real     < right._real;
    case type_real:                                return left._real     < right._real;
    case type_string:   left.cast (type_string);   return left._string   < right._string;
    case type_date:     left.cast (type_date);     return left._date     < right._date;
    case type_duration: left.cast (type_duration); return left._duration < right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string < right._string;

    case type_string:
      if (left.source () == "priority" || right.source () == "priority")
      {
             if (left._string != "H" && right._string == "H") return true;
        else if (left._string == "L" && right._string == "M") return true;
        else if (left._string == ""  && right._string != "")  return true;
        else                                                  return false;
      }
      else
      {
        if (left.trivial () || right.trivial ())
          return false;

        return left._string < right._string;
      }

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date < right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration < right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date < right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration < right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator<= (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:                             return !left._bool || right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  <= right._integer;
    case type_real:     left.cast (type_real);     return left._real     <= right._real;
    case type_string:   left.cast (type_string);   return left._string   <= right._string;
    case type_date:     left.cast (type_date);     return left._date     <= right._date;
    case type_duration: left.cast (type_duration); return left._duration <= right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_integer); return left._integer  <= right._integer;
    case type_integer:                             return left._integer  <= right._integer;
    case type_real:     left.cast (type_real);     return left._real     <= right._real;
    case type_string:   left.cast (type_string);   return left._string   <= right._string;
    case type_date:     left.cast (type_date);     return left._date     <= right._date;
    case type_duration: left.cast (type_duration); return left._duration <= right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_real);    return left._real     <= right._real;
    case type_integer:  right.cast (type_real);    return left._real     <= right._real;
    case type_real:                                return left._real     <= right._real;
    case type_string:   left.cast (type_string);   return left._string   <= right._string;
    case type_date:     left.cast (type_date);     return left._date     <= right._date;
    case type_duration: left.cast (type_duration); return left._duration <= right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string <= right._string;

    case type_string:
      if (left.source () == "priority" || right.source () == "priority")
      {
             if (left._string        == right._string       ) return true;
        else if (                       right._string == "H") return true;
        else if (left._string == "L" && right._string == "M") return true;
        else if (left._string == ""                         ) return true;
        else                                                  return false;
      }
      else
      {
        if (left.trivial () || right.trivial ())
          return false;

        return left._string <= right._string;
      }

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date <= right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration <= right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date <= right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration <= right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator> (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:                             return !left._bool && right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  > right._integer;
    case type_real:     left.cast (type_real);     return left._real     > right._real;
    case type_string:   left.cast (type_string);   return left._string   > right._string;
    case type_date:     left.cast (type_date);     return left._date     > right._date;
    case type_duration: left.cast (type_duration); return left._duration > right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_integer); return left._integer  > right._integer;
    case type_integer:                             return left._integer  > right._integer;
    case type_real:     left.cast (type_real);     return left._real     > right._real;
    case type_string:   left.cast (type_string);   return left._string   > right._string;
    case type_date:     left.cast (type_date);     return left._date     > right._date;
    case type_duration: left.cast (type_duration); return left._duration > right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_real);    return left._real     > right._real;
    case type_integer:  right.cast (type_real);    return left._real     > right._real;
    case type_real:                                return left._real     > right._real;
    case type_string:   left.cast (type_string);   return left._string   > right._string;
    case type_date:     left.cast (type_date);     return left._date     > right._date;
    case type_duration: left.cast (type_duration); return left._duration > right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string > right._string;

    case type_string:
      if (left.source () == "priority" || right.source () == "priority")
      {
             if (left._string == "H" && right._string != "H") return true;
        else if (left._string == "M" && right._string == "L") return true;
        else if (left._string != ""  && right._string == "")  return true;
        else                                                  return false;
      }
      else
      {
        if (left.trivial () || right.trivial ())
          return false;

        return left._string> right._string;
      }
    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date > right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration > right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date > right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration > right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator>= (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:                             return left._bool || !right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer  >= right._integer;
    case type_real:     left.cast (type_real);     return left._real     >= right._real;
    case type_string:   left.cast (type_string);   return left._string   >= right._string;
    case type_date:     left.cast (type_date);     return left._date     >= right._date;
    case type_duration: left.cast (type_duration); return left._duration >= right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_integer); return left._integer  >= right._integer;
    case type_integer:                             return left._integer  >= right._integer;
    case type_real:     left.cast (type_real);     return left._real     >= right._real;
    case type_string:   left.cast (type_string);   return left._string   >= right._string;
    case type_date:     left.cast (type_date);     return left._date     >= right._date;
    case type_duration: left.cast (type_duration); return left._duration >= right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_CMP_UNKNOWN);
    case type_boolean:  right.cast (type_real);    return left._real     >= right._real;
    case type_integer:  right.cast (type_real);    return left._real     >= right._real;
    case type_real:                                return left._real     >= right._real;
    case type_string:   left.cast (type_string);   return left._string   >= right._string;
    case type_date:     left.cast (type_date);     return left._date     >= right._date;
    case type_duration: left.cast (type_duration); return left._duration >= right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_string);
      return left._string >= right._string;

    case type_string:
      if (left.source () == "priority" || right.source () == "priority")
      {
             if (left._string        == right._string       ) return true;
        else if (left._string == "H"                        ) return true;
        else if (left._string == "M" && right._string == "L") return true;
        else if (                       right._string == "" ) return true;
        else                                                  return false;
      }
      else
      {
        if (left.trivial () || right.trivial ())
          return false;

        return left._string >= right._string;
      }

    case type_date:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_date);
      return left._date >= right._date;

    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      left.cast (type_duration);
      return left._duration >= right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_date);
      return left._date >= right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_CMP_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      if (left.trivial () || right.trivial ())
        return false;

      right.cast (type_duration);
      return left._duration >= right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator== (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_EQ_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_EQ_UNKNOWN);
    case type_boolean:                             return left._bool == right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real == right._real;
    case type_string:   left.cast (type_string);   return left._string == right._string;
    case type_date:     left.cast (type_date);     return left._date == right._date;
    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_EQ_UNKNOWN);
    case type_boolean:  right.cast (type_integer); return left._integer == right._integer;
    case type_integer:                             return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real == right._real;
    case type_string:   left.cast (type_string);   return left._string == right._string;
    case type_date:     left.cast (type_date);     return left._date == right._date;
    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_EQ_UNKNOWN);
    case type_boolean:  right.cast (type_real);    return left._real == right._real;
    case type_integer:  right.cast (type_real);    return left._real == right._real;
    case type_real:                                return left._real == right._real;
    case type_string:   left.cast (type_string);   return left._string == right._string;
    case type_date:     left.cast (type_date);     return left._date == right._date;
    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_EQ_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
      right.cast (type_string);

      // Status is always compared caseless.
      if (left.source () == "status")
        return compare (left._string, right._string, false);

      return left._string == right._string;

    case type_date:
      left.cast (type_date);
      return left._date == right._date;

    case type_duration:
      left.cast (type_duration);
      return left._duration == right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_EQ_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      right.cast (type_date);
      return left._date == right._date;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_EQ_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      right.cast (type_duration);
      return left._duration == right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator!= (const Variant& other) const
{
  return !(*this == other);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_match (const Variant& other, const Task& task) const
{
  // Simple matching case first.
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  left.cast (type_string);
  right.cast (type_string);

  std::string pattern = right._string;
  Lexer::dequote (pattern);

  if (searchUsingRegex)
  {
    RX r (pattern, searchCaseSensitive);
    if (r.match (left._string))
      return true;

    // If the above did not match, and the left source is "description", then
    // in the annotations.
    if (left.source () == "description")
    {
      std::map <std::string, std::string> annotations;
      task.getAnnotations (annotations);

      std::map <std::string, std::string>::iterator a;
      for (a = annotations.begin (); a != annotations.end (); ++a)
        if (r.match (a->second))
          return true;
    }
  }
  else
  {
    if (find (left._string, pattern, searchCaseSensitive) != std::string::npos)
      return true;

    // If the above did not match, and the left source is "description", then
    // in the annotations.
    if (left.source () == "description")
    {
      std::map <std::string, std::string> annotations;
      task.getAnnotations (annotations);

      std::map <std::string, std::string>::iterator a;
      for (a = annotations.begin (); a != annotations.end (); ++a)
        if (find (a->second, pattern, searchCaseSensitive) != std::string::npos)
          return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_nomatch (const Variant& other, const Task& task) const
{
  return ! operator_match (other, task);
}

////////////////////////////////////////////////////////////////////////////////
// Partial match is mostly a clone of operator==, but with some overrides:
//
//   date <partial> date     --> same day check
//   string <partial> string --> leftmost
//
bool Variant::operator_partial (const Variant& other) const
{
  Variant left (*this);
  Variant right (other);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (left._type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_EQ_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_EQ_UNKNOWN);
    case type_boolean:                             return left._bool    == right._bool;
    case type_integer:  left.cast (type_integer);  return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real    == right._real;
    case type_string:   left.cast (type_string);   return left._string  == right._string;

    // Same-day comparison.
    case type_date:
      {
        left.cast (type_date);
        Date left_date (left._date);
        Date right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_EQ_UNKNOWN);
    case type_boolean:  right.cast (type_integer); return left._integer == right._integer;
    case type_integer:                             return left._integer == right._integer;
    case type_real:     left.cast (type_real);     return left._real    == right._real;
    case type_string:   left.cast (type_string);   return left._string  == right._string;

    // Same-day comparison.
    case type_date:
      {
        left.cast (type_date);
        Date left_date (left._date);
        Date right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration: left.cast (type_duration); return left._duration == right._duration;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_EQ_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_real);
      return left._real == right._real;

    case type_string:
      left.cast (type_string);
      return left._string == right._string;

    // Same-day comparison.
    case type_date:
      {
        left.cast (type_date);
        Date left_date (left._date);
        Date right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration:
      left.cast (type_duration);
      return left._duration == right._duration;
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_EQ_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
      {
        right.cast (type_string);

        // Status is always compared caseless.
        if (left.source () == "status")
          return compare (left._string, right._string, false);

        int left_len  = left._string.length ();
        int right_len = right._string.length ();

        if ((left_len == 0 && right_len != 0) ||
            (left_len != 0 && right_len == 0))
          return false;

        // Dodgy.
        if (left._string.length () < right._string.length ())
          return false;

        return left._string.substr (0, right._string.length ()) == right._string;
      }

    // Same-day comparison.
    case type_date:
      {
        left.cast (type_date);
        Date left_date (left._date);
        Date right_date (right._date);
        return left_date.sameDay (right_date);
      }

    case type_duration:
      left.cast (type_duration);
      return left._duration == right._duration;
    }
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_EQ_UNKNOWN);

    // Same-day comparison.
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      {
        right.cast (type_date);
        Date left_date (left._date);
        Date right_date (right._date);
        return left_date.sameDay (right_date);
      }
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_EQ_UNKNOWN);

    // Same-day comparison.
    case type_boolean:
    case type_integer:
    case type_real:
    case type_string:
    case type_date:
    case type_duration:
      right.cast (type_duration);
      return left._duration == right._duration;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Inverse of operator_partial.
bool Variant::operator_nopartial (const Variant& other) const
{
  return ! operator_partial (other);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_hastag (const Variant& other, const Task& task) const
{
  Variant right (other);
  right.cast (type_string);
  Lexer::dequote (right._string);
  return task.hasTag (right._string);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator_notag (const Variant& other, const Task& task) const
{
  return ! operator_hastag (other, task);
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::operator! () const
{
  Variant left (*this);

  if (left._type == type_string)
    Lexer::dequote (left._string);

  left.cast (type_boolean);
  return ! left._bool;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator^= (const Variant& other)
{
  switch (_type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_EXP_UNKNOWN);
    break;

  case type_boolean:
    throw std::string (STRING_VARIANT_EXP_BOOL);
    break;

  case type_integer:
    switch (other._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_EXP_UNKNOWN);
    case type_boolean:  throw std::string (STRING_VARIANT_EXP_BOOL);
    case type_integer:  _integer = (int) pow (static_cast<double>(_integer), static_cast<double>(other._integer)); break;
    case type_real:     throw std::string (STRING_VARIANT_EXP_NON_INT);
    case type_string:   throw std::string (STRING_VARIANT_EXP_STRING);
    case type_date:     throw std::string (STRING_VARIANT_EXP_DATE);
    case type_duration: throw std::string (STRING_VARIANT_EXP_DURATION);
    }
    break;

  case type_real:
    switch (other._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_EXP_UNKNOWN);
    case type_boolean:  throw std::string (STRING_VARIANT_EXP_BOOL);
    case type_integer:  _real = pow (_real, static_cast<double>(other._integer)); break;
    case type_real:     throw std::string (STRING_VARIANT_EXP_NON_INT);
    case type_string:   throw std::string (STRING_VARIANT_EXP_STRING);
    case type_date:     throw std::string (STRING_VARIANT_EXP_DATE);
    case type_duration: throw std::string (STRING_VARIANT_EXP_DURATION);
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_EXP_STRING);
    break;

  case type_date:
    throw std::string (STRING_VARIANT_EXP_DATE);
    break;

  case type_duration:
    throw std::string (STRING_VARIANT_EXP_DURATION);
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator^ (const Variant& other) const
{
  Variant left (*this);
  left ^= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator-= (const Variant& other)
{
  Variant right (other);

  switch (_type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_SUB_UNKNOWN);
    break;

  case type_boolean:
    throw std::string (STRING_VARIANT_SUB_BOOL);
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_SUB_UNKNOWN);
    case type_boolean:  right.cast (type_integer); _integer  -= right._integer;   break;
    case type_integer:                             _integer  -= right._integer;   break;
    case type_real:     cast (type_real);          _real     -= right._real;      break;
    case type_string:   throw std::string (STRING_VARIANT_SUB_STRING);
    case type_date:     cast (type_date);          _date     -= right._date;      break;
    case type_duration: cast (type_duration);      _duration -= right._duration;  break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_SUB_UNKNOWN);

    case type_string:
      throw std::string (STRING_VARIANT_SUB_STRING);

    case type_boolean:
    case type_integer:
    case type_real:
    case type_date:
    case type_duration:
      right.cast (type_real);
      _real -= right._real;
      break;
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_SUB_STRING);
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_SUB_UNKNOWN);
    case type_boolean:  right.cast (type_integer); _date -= right._integer;    break;
    case type_integer:                             _date -= right._integer;    break;
    case type_real:                                _date -= (int) right._real; break;
    case type_string:   throw std::string (STRING_VARIANT_SUB_STRING);
    case type_date:     cast (type_duration);      _duration -= right._date;   break;
    case type_duration:                            _date -= right._duration;   break;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_SUB_UNKNOWN);
    case type_boolean:  right.cast (type_integer); _duration -= right._integer;    break;
    case type_integer:                             _duration -= right._integer;    break;
    case type_real:                                _duration -= (int) right._real; break;
    case type_string:   throw std::string (STRING_VARIANT_SUB_STRING);
    case type_date:     throw std::string (STRING_VARIANT_SUB_DATE);
    case type_duration:                            _duration -= right._duration;   break;
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator- (const Variant& other) const
{
  Variant left (*this);
  left -= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator+= (const Variant& other)
{
  Variant right (other);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (_type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_ADD_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_ADD_UNKNOWN);
    case type_boolean:  throw std::string (STRING_VARIANT_ADD_BOOL);
    case type_integer:  cast (type_integer);  _integer  += right._integer;  break;
    case type_real:     cast (type_real);     _real     += right._real;     break;
    case type_string:   cast (type_string);   _string   += right._string;   break;
    case type_date:     cast (type_date);     _date     += right._date;     break;
    case type_duration: cast (type_duration); _duration += right._duration; break;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_ADD_UNKNOWN);
    case type_boolean:  right.cast (type_integer); _integer  += right._integer;   break;
    case type_integer:                             _integer  += right._integer;   break;
    case type_real:     cast (type_real);          _real     += right._real;      break;
    case type_string:   cast (type_string);        _string   += right._string;    break;
    case type_date:     cast (type_date);          _date     += right._date;      break;
    case type_duration: cast (type_duration);      _duration += right._duration;  break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_ADD_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_real);
      _real += right._real;
      break;

    case type_string:
      cast (type_string);
      _string += right._string;
      break;

    case type_date:
      _type = type_date;
      _date = (unsigned) (int) _real + right._date;
      break;

    case type_duration:
      _type = type_duration;
      _duration = (unsigned) (int) _real + right._duration;
      break;
    }
    break;

  case type_string:
    _string += (std::string) right;
    break;

  case type_date:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_ADD_UNKNOWN);
    case type_boolean:  right.cast (type_date); _date += right._date;       break;
    case type_integer:                          _date += right._integer;    break;
    case type_real:                             _date += (int) right._real; break;
    case type_string:   cast (type_string);     _string += right._string;   break;
    case type_date:     throw std::string (STRING_VARIANT_ADD_DATE);
    case type_duration:                         _date += right._duration;   break;
    }
    break;

  case type_duration:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_ADD_UNKNOWN);
    case type_boolean:  right.cast (type_duration); _duration += right._duration;   break;
    case type_integer:                              _duration += right._integer;    break;
    case type_real:                                 _duration += (int) right._real; break;
    case type_string:   cast (type_string);         _string += right._string;       break;
    case type_date:     cast (type_date);           _date += right._date;           break;
    case type_duration:                             _duration += right._duration;   break;
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator+ (const Variant& other) const
{
  Variant left (*this);
  left += other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator*= (const Variant& other)
{
  Variant right (other);

  if (right._type == type_string)
    Lexer::dequote (right._string);

  switch (_type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_MUL_UNKNOWN);
    break;

  case type_boolean:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_MUL_UNKNOWN);
    case type_boolean:  throw std::string (STRING_VARIANT_MUL_BOOL);
    case type_integer:  cast (type_integer);       _integer  *= right._integer;      break;
    case type_real:     cast (type_real);          _real     *= right._real;         break;
    case type_string:   _string = (_bool ? right._string : ""); _type = type_string; break;
    case type_date:     throw std::string (STRING_VARIANT_MUL_DATE);
    case type_duration: cast (type_duration);      _duration *= right._duration;     break;
    }
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_MUL_UNKNOWN);
    case type_boolean:  right.cast (type_integer); _integer  *= right._integer;   break;
    case type_integer:                             _integer  *= right._integer;   break;
    case type_real:     cast (type_real);          _real     *= right._real;      break;
    case type_string:
      {
        int limit = _integer;
        // assert (limit < 128);
        _type = type_string;
        _string = "";
        while (limit--)
          _string += right._string;
      }
      break;
    case type_date:     throw std::string (STRING_VARIANT_MUL_DATE);
    case type_duration: cast (type_duration);      _duration *= right._duration;  break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_MUL_UNKNOWN);

    case type_boolean:
    case type_integer:
    case type_real:
      right.cast (type_real);
      _real *= right._real;
      break;

    case type_string:
      throw std::string (STRING_VARIANT_MUL_REAL_STR);

    case type_date:
      throw std::string (STRING_VARIANT_MUL_DATE);

    case type_duration:
      _type = type_duration;
      _duration = (time_t) (unsigned) (int) (_real * static_cast<double>(right._duration));
    }
    break;

  case type_string:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_MUL_UNKNOWN);
    case type_boolean:  if (! right._bool) _string = ""; break;
    case type_integer:
      {
        int limit = right._integer - 1;
        // assert (limit < 128);
        std::string fragment = _string;
        while (limit--)
          _string += fragment;
      }
      break;
    case type_real:     throw std::string (STRING_VARIANT_MUL_STR_REAL);
    case type_string:   throw std::string (STRING_VARIANT_MUL_STR_STR);
    case type_date:     throw std::string (STRING_VARIANT_MUL_STR_DATE);
    case type_duration: throw std::string (STRING_VARIANT_MUL_STR_DUR);
    }
    break;

  case type_date:
    throw std::string (STRING_VARIANT_MUL_DATE);

  case type_duration:
    switch (right._type)
    {
    case type_unknown:  throw std::string (STRING_VARIANT_MUL_UNKNOWN);
    case type_boolean:  right.cast (type_duration); _duration *= right._duration;   break;
    case type_integer:                              _duration *= right._integer;    break;
    case type_real:
      _duration = (time_t) (unsigned) (int) (static_cast<double>(_duration) * right._real);
      break;
    case type_string:   throw std::string (STRING_VARIANT_MUL_DUR_STR);
    case type_date:     throw std::string (STRING_VARIANT_MUL_DUR_DATE);
    case type_duration: throw std::string (STRING_VARIANT_MUL_DUR_DUR);
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator* (const Variant& other) const
{
  Variant left (*this);
  left *= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator/= (const Variant& other)
{
  Variant right (other);

  switch (_type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_DIV_UNKNOWN);
    break;

  case type_boolean:
    throw std::string (STRING_VARIANT_DIV_BOOL);
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_DIV_UNKNOWN);

    case type_boolean:
      throw std::string (STRING_VARIANT_DIV_INT_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _integer /= right._integer;
      break;

    case type_real:
      if (right._real == 0.0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      cast (type_real);
      _real /= right._real;
      break;

    case type_string:
      throw std::string (STRING_VARIANT_DIV_INT_STR);

    case type_date:
      throw std::string (STRING_VARIANT_DIV_INT_DATE);

    case type_duration:
      if (right._duration == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _type = type_duration;
      _duration = (time_t) (unsigned) (int) (_integer / right._duration);
      break;
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_DIV_UNKNOWN);

    case type_boolean:
      throw std::string (STRING_VARIANT_DIV_REAL_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _real /= static_cast<double>(right._integer);
      break;

    case type_real:
      if (right._real == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _real /= right._real;
      break;

    case type_string:
      throw std::string (STRING_VARIANT_DIV_REAL_STR);

    case type_date:
      throw std::string (STRING_VARIANT_DIV_REAL_DATE);

    case type_duration:
      if (right._duration == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _type = type_duration;
      _duration = (time_t) (unsigned) (int) (_real / right._duration);
      break;
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_DIV_REAL_STR);
    break;

  case type_date:
    throw std::string (STRING_VARIANT_DIV_REAL_DATE);

  case type_duration:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_DIV_UNKNOWN);

    case type_boolean:
      throw std::string (STRING_VARIANT_DIV_DUR_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _duration /= right._integer;
      break;

    case type_real:
      if (right._real == 0)
        throw std::string (STRING_VARIANT_DIV_ZERO);
      _duration = (time_t) (unsigned) (int) (static_cast<double>(_duration) / right._real);
      break;

    case type_string:
      throw std::string (STRING_VARIANT_DIV_DUR_STR);

    case type_date:
      throw std::string (STRING_VARIANT_DIV_DUR_DATE);

    case type_duration:
      throw std::string (STRING_VARIANT_DIV_DUR_DUR);
    }
    break;
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator/ (const Variant& other) const
{
  Variant left (*this);
  left /= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant& Variant::operator%= (const Variant& other)
{
  Variant right (other);

  switch (_type)
  {
  case type_unknown:
    throw std::string (STRING_VARIANT_MOD_UNKNOWN);
    break;

  case type_boolean:
    throw std::string (STRING_VARIANT_MOD_BOOL);
    break;

  case type_integer:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_MOD_UNKNOWN);

    case type_boolean:
      throw std::string (STRING_VARIANT_MOD_INT_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      _integer %= right._integer;
      break;

    case type_real:
      if (right._real == 0.0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      cast (type_real);
      _real = fmod (_real, right._real);
      break;

    case type_string:
      throw std::string (STRING_VARIANT_MOD_INT_STR);

    case type_date:
      throw std::string (STRING_VARIANT_MOD_INT_DATE);

    case type_duration:
      throw std::string (STRING_VARIANT_MOD_INT_DUR);
    }
    break;

  case type_real:
    switch (right._type)
    {
    case type_unknown:
      throw std::string (STRING_VARIANT_MOD_UNKNOWN);

    case type_boolean:
      throw std::string (STRING_VARIANT_MOD_REAL_BOOL);

    case type_integer:
      if (right._integer == 0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      _real = fmod (_real, static_cast<double>(right._integer));
      break;

    case type_real:
      if (right._real == 0)
        throw std::string (STRING_VARIANT_MOD_ZERO);
      _real = fmod (_real, right._real);
      break;

    case type_string:
      throw std::string (STRING_VARIANT_MOD_REAL_STR);

    case type_date:
      throw std::string (STRING_VARIANT_MOD_REAL_DATE);

    case type_duration:
      throw std::string (STRING_VARIANT_MOD_REAL_DUR);
    }
    break;

  case type_string:
    throw std::string (STRING_VARIANT_MOD_STR);

  case type_date:
    throw std::string (STRING_VARIANT_MOD_DATE);

  case type_duration:
    throw std::string (STRING_VARIANT_MOD_DUR);
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Variant Variant::operator% (const Variant& other) const
{
  Variant left (*this);
  left %= other;
  return left;
}

////////////////////////////////////////////////////////////////////////////////
Variant::operator std::string () const
{
  switch (_type)
  {
  case type_boolean:
    return std::string (_bool ? "true" : "false");

  case type_integer:
    {
      std::stringstream s;
      s << _integer;
      return s.str ();
    }

  case type_real:
    {
      std::stringstream s;
      s << _real;
      return s.str ();
    }

  case type_string:
    return _string;

  case type_date:
    {
      struct tm* t = localtime (&_date);

      std::stringstream s;
      s.width (4);
      s << t->tm_year + 1900;
      s << '-';
      s.width (2);
      s.fill ('0');
      s << t->tm_mon + 1;
      s << '-';
      s.width (2);
      s.fill ('0');
      s << t->tm_mday;
      s << 'T';
      s.width (2);
      s.fill ('0');
      s << t->tm_hour;
      s << ':';
      s.width (2);
      s.fill ('0');
      s << t->tm_min;
      s << ':';
      s.width (2);
      s.fill ('0');
      s << t->tm_sec;
      return s.str ();
    }

  case type_duration:
    {
      time_t t = _duration;
      if (t)
      {
        std::stringstream s;
        s << 'P';

        int years = t / (365 * 86400);  t -= years * 365 * 86400;
        int days  = t / 86400;          t -= days        * 86400;

        if (years) s << years << 'Y';
        if (days)  s << days  << 'D';

        int hours   = t / 3600;           t -= hours   * 3600;
        int minutes = t /   60;           t -= minutes *   60;
        int seconds = t;

        if (hours || minutes || seconds)
        {
          s << 'T';
          if (hours)   s << hours   << 'H';
          if (minutes) s << minutes << 'M';
          if (seconds) s << seconds << 'S';
        }

        return s.str ();
      }
      else
      {
        return "P0S";
      }
    }

  case type_unknown:
    throw std::string (STRING_VARIANT_RENDER_UNK);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
void Variant::sqrt ()
{
  if (_type == type_string)
    Lexer::dequote (_string);

  cast (type_real);
  if (_real < 0.0)
    throw std::string (STRING_VARIANT_SQRT_NEG);
  _real = ::sqrt (_real);
}

////////////////////////////////////////////////////////////////////////////////
void Variant::cast (const enum type new_type)
{
  // Short circuit.
  if (_type == new_type)
    return;

  if (_type == type_unknown || new_type == type_unknown)
    throw std::string (STRING_VARIANT_COERCE_UNK);

  // From type_boolean
  switch (_type)
  {
  case type_unknown:
    break;

  case type_boolean:
    switch (new_type)
    {
    case type_unknown:                                        break;
    case type_boolean:                                        break;
    case type_integer:  _integer  = _bool ? 1 : 0;            break;
    case type_real:     _real     = _bool ? 1.0 : 0.0;        break;
    case type_string:   _string   = _bool ? "true" : "false"; break;
    case type_date:     _date     = _bool ? 1 : 0;            break;
    case type_duration: _duration = _bool ? 1 : 0;            break;
    }
    break;

  case type_integer:
    switch (new_type)
    {
    case type_unknown:                                         break;
    case type_boolean:  _bool = _integer == 0 ? false : true;  break;
    case type_integer:                                         break;
    case type_real:     _real = static_cast<double>(_integer); break;
    case type_string:
      {
        char temp[24];
        sprintf (temp, "%d", _integer);
        _string = temp;
      }
      break;
    case type_date:     _date = (time_t) _integer;            break;
    case type_duration: _duration = (time_t) _integer;        break;
    }
    break;

  case type_real:
    switch (new_type)
    {
    case type_unknown:                                       break;
    case type_boolean:  _bool = _real == 0.0 ? false : true; break;
    case type_integer:  _integer = (int) _real;              break;
    case type_real:                                          break;
    case type_string:
      {
        char temp[24];
        sprintf (temp, "%g", _real);
        _string = temp;
      }
      break;
    case type_date:     _date = (time_t) (int) _real;        break;
    case type_duration: _duration = (time_t) (int) _real;    break;
    }
    break;

  case type_string:
    Lexer::dequote (_string);
    switch (new_type)
    {
    case type_unknown:                                                        break;
    case type_boolean:
      _bool = (_string.length () == 0 ||
               _string == "0"         ||
               _string == "0.0") ? false : true;
      break;
    case type_integer:
      _integer = (int) strtol (_string.c_str (), NULL, (_string.substr (0, 2) == "0x" ? 16 : 10));
      break;
    case type_real:     _real = strtod (_string.c_str (), NULL);              break;
    case type_string:                                                         break;
    case type_date:
      {
        _date = 0;

        ISO8601d iso;
        std::string::size_type pos = 0;
        if (isoEnabled               &&
            iso.parse (_string, pos) &&
            pos == _string.length ())
        {
          _date = (time_t) iso;
        }
        else if (dateFormat != "")
        {
          Date d (_string, dateFormat);
          _date = d.toEpoch ();
        }
      }
      break;
    case type_duration:
      {
        _duration = 0;
        Duration dur;
        std::string::size_type pos = 0;
        if (dur.parse (_string, pos) &&
            pos == _string.length ())
        {
          _duration = (time_t) dur;
        }
        else
        {
          ISO8601p iso;
          pos = 0;
          if (iso.parse (_string, pos) &&
              pos == _string.length ())
          {
            _duration = (time_t) iso;
          }
        }
      }
      break;
    }
    break;

  case type_date:
    switch (new_type)
    {
    case type_unknown:                                      break;
    case type_boolean:  _bool = _date != 0 ? true : false;  break;
    case type_integer:  _integer = (int) _date;             break;
    case type_real:     _real = static_cast<double>(_date); break;
    case type_string:   _string = (std::string) *this;      break;
    case type_date:                                         break;
    case type_duration: _duration = _date;                  break;
    }
    break;

  case type_duration:
    switch (new_type)
    {
    case type_unknown:                                          break;
    case type_boolean:  _bool = _duration != 0 ? true : false;  break;
    case type_integer:  _integer = (int) _duration;             break;
    case type_real:     _real = static_cast<double>(_duration); break;
    case type_string:   _string = (std::string) *this;          break;
    case type_date:     _date = _duration;                      break;
    case type_duration:                                         break;
    }
    break;
  }

  _type = new_type;
}

////////////////////////////////////////////////////////////////////////////////
int Variant::type ()
{
  return _type;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::trivial () const
{
  return (_type == type_unknown)                      ||
         (_type == type_integer  && _integer  == 0)   ||
         (_type == type_real     && _real     == 0.0) ||
         (_type == type_string   && _string   == "")  ||
         (_type == type_date     && _date     == 0)   ||
         (_type == type_duration && _duration == 0);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Variant::get_bool () const
{
  return _bool;
}

////////////////////////////////////////////////////////////////////////////////
int Variant::get_integer () const
{
  return _integer;
}

////////////////////////////////////////////////////////////////////////////////
double Variant::get_real () const
{
  return _real;
}

////////////////////////////////////////////////////////////////////////////////
std::string Variant::get_string () const
{
  return _string;
}

////////////////////////////////////////////////////////////////////////////////
time_t Variant::get_date () const
{
  return _date;
}

////////////////////////////////////////////////////////////////////////////////
time_t Variant::get_duration () const
{
  return _duration;
}

////////////////////////////////////////////////////////////////////////////////
