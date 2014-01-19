////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2014, Paul Beckingham, Federico Hernandez.
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

#include <utf8.h>
#include <ISO8601.h>
#include <Duration.h>
#include <Lexer.h>

////////////////////////////////////////////////////////////////////////////////
Lexer::Lexer (const std::string& input)
: _input (input)
, _i (0)
, _n0 (32)
, _n1 (32)
, _n2 (32)
, _n3 (32)
, _ambiguity (true)
{
  // Read 4 chars in preparation.  Even if there are < 4.  Take a deep breath.
  shift ();
  shift ();
  shift ();
  shift ();
}

////////////////////////////////////////////////////////////////////////////////
Lexer::~Lexer ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Walk the input string, looking for transitions.
bool Lexer::token (std::string& token, Type& type)
{
  // Start with nothing.
  token = "";

  // Different types of matching quote:  ', ".
  int quote = 0;

  type = typeNone;
  while (_n0)
  {
    switch (type)
    {
    case typeNone:
      if (is_ws (_n0))
        shift ();
      else if (_n0 == '"' || _n0 == '\'')
      {
        type = typeString;
        quote = _n0;
        shift ();
      }
      else if (_n0 == '0' &&
               _n1 == 'x' &&
               is_hex_digit (_n2))
      {
        type = typeHex;
        token += utf8_character (_n0);
        shift ();
        token += utf8_character (_n0);
        shift ();
        token += utf8_character (_n0);
        shift ();
      }
      else if (is_dec_digit (_n0))
      {
        // Speculatively try a date and duration parse.  Longest wins.
        std::string::size_type iso_i = 0;
        std::string iso_token;
        ISO8601d iso;
        iso.ambiguity (_ambiguity);
        if (iso.parse (_input.substr (_i < 4 ? 0 : _i - 4), iso_i))
          iso_token = _input.substr ((_i < 4 ? 0 : _i - 4), iso_i);

        std::string::size_type dur_i = 0;
        std::string dur_token;
        Duration dur;
        if (dur.parse (_input.substr (_i < 4 ? 0 : _i - 4), dur_i))
          dur_token = _input.substr ((_i < 4 ? 0 : _i - 4), dur_i);

        if (iso_token.length () > dur_token.length ())
        {
          while (iso_i--) shift ();
          token = iso_token;
          type = typeDate;
          return true;
        }
        else if (dur_token.length () > iso_token.length ())
        {
          while (dur_i--) shift ();
          token = dur_token;
          type = typeDuration;
          return true;
        }

        type = typeNumber;
        token += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == '.' && is_dec_digit (_n1))
      {
        type = typeDecimal;
        token += utf8_character (_n0);
        shift ();
      }
      else if (is_triple_op (_n0, _n1, _n2))
      {
        type = typeOperator;
        token += utf8_character (_n0);
        shift ();
        token += utf8_character (_n0);
        shift ();
        token += utf8_character (_n0);
        shift ();
        return true;
      }
      else if (is_double_op (_n0, _n1))
      {
        type = typeOperator;
        token += utf8_character (_n0);
        shift ();
        token += utf8_character (_n0);
        shift ();
        return true;
      }
      else if (is_single_op (_n0))
      {
        type = typeOperator;
        token += utf8_character (_n0);
        shift ();
        return true;
      }
      else if (_n0 == '\\')
      {
        type = typeIdentifierEscape;
        shift ();
      }
      else if (is_ident_start (_n0))
      {
        // Speculatively try a date and duration parse.  Longest wins.
        std::string::size_type iso_i = 0;
        std::string iso_token;
        ISO8601p iso;
        if (iso.parse (_input.substr (_i < 4 ? 0 : _i - 4), iso_i))
          iso_token = _input.substr ((_i < 4 ? 0 : _i - 4), iso_i);

        std::string::size_type dur_i = 0;
        std::string dur_token;
        Duration dur;
        if (dur.parse (_input.substr (_i < 4 ? 0 : _i - 4), dur_i))
          dur_token = _input.substr ((_i < 4 ? 0 : _i - 4), dur_i);

        if (iso_token.length () > dur_token.length ())
        {
          while (iso_i--) shift ();
          token = iso_token;
          type = typeDuration;
          return true;
        }
        else if (dur_token.length () > iso_token.length ())
        {
          while (dur_i--) shift ();
          token = dur_token;
          type = typeDuration;
          return true;
        }

        type = typeIdentifier;
        token += utf8_character (_n0);
        shift ();
      }
      else
        throw std::string ("Unexpected error 1");
      break;

    case typeString:
      if (_n0 == quote)
      {
        shift ();
        quote = 0;
        return true;
      }
      else if (_n0 == '\\')
      {
        type = typeEscape;
        shift ();
      }
      else
      {
        token += utf8_character (_n0);
        shift ();
      }
      break;

    case typeIdentifier:
      if (is_ident (_n0))
      {
        token += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    case typeIdentifierEscape:
      if (_n0 == 'u')
      {
        type = typeEscapeUnicode;
        shift ();
      }
      break;

    case typeEscape:
      if (_n0 == 'x')
      {
        type = typeEscapeHex;
        shift ();
      }
      else if (_n0 == 'u')
      {
        type = typeEscapeUnicode;
        shift ();
      }
      else
      {
        token += decode_escape (_n0);
        type = quote ? typeString : typeIdentifier;
        shift ();
      }
      break;

    case typeEscapeHex:
      if (is_hex_digit (_n0) && is_hex_digit (_n1))
      {
        token += utf8_character (hex_to_int (_n0, _n1));
        type = quote ? typeString : typeIdentifier;
        shift ();
        shift ();
      }
      else
      {
        type = quote ? typeString : typeIdentifier;
        shift ();
        quote = 0;
        return true;
      }
      break;

    case typeEscapeUnicode:
      if (is_hex_digit (_n0) &&
          is_hex_digit (_n1) &&
          is_hex_digit (_n2) &&
          is_hex_digit (_n3))
      {
        token += utf8_character (hex_to_int (_n0, _n1, _n2, _n3));
        shift ();
        shift ();
        shift ();
        shift ();
        type = quote ? typeString : typeIdentifier;
      }
      else if (_n0 == quote)
      {
        type = typeString;
        shift ();
        quote = 0;
        return true;
      }

    case typeNumber:
      if (is_dec_digit (_n0))
      {
        token += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == '.')
      {
        type = typeDecimal;
        token += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == 'e' || _n0 == 'E')
      {
        type = typeExponentIndicator;
        token += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    case typeDecimal:
      if (is_dec_digit (_n0))
      {
        token += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == 'e' || _n0 == 'E')
      {
        type = typeExponentIndicator;
        token += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    case typeExponentIndicator:
      if (_n0 == '+' || _n0 == '-')
      {
        token += utf8_character (_n0);
        shift ();
      }
      else if (is_dec_digit (_n0))
      {
        type = typeExponent;
        token += utf8_character (_n0);
        shift ();
      }
      break;

    case typeExponent:
      if (is_dec_digit (_n0))
      {
        token += utf8_character (_n0);
        shift ();
      }
      else if (_n0 == '.')
      {
        token += utf8_character (_n0);
        shift ();
      }
      else
      {
        type = typeDecimal;
        return true;
      }
      break;

    case typeHex:
      if (is_hex_digit (_n0))
      {
        token += utf8_character (_n0);
        shift ();
      }
      else
      {
        return true;
      }
      break;

    default:
      throw std::string ("Unexpected error 2");
      break;
    }

    // Fence post.
    if (!_n0 && token != "")
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::ambiguity (bool value)
{
  _ambiguity = value;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Lexer::type_name (const Type& type)
{
  switch (type)
  {
  case Lexer::typeNone:              return "None";
  case Lexer::typeString:            return "String";
  case Lexer::typeIdentifier:        return "Identifier";
  case Lexer::typeIdentifierEscape:  return "IdentifierEscape";
  case Lexer::typeNumber:            return "Number";
  case Lexer::typeDecimal:           return "Decimal";
  case Lexer::typeExponentIndicator: return "ExponentIndicator";
  case Lexer::typeExponent:          return "Exponent";
  case Lexer::typeHex:               return "Hex";
  case Lexer::typeOperator:          return "Operator";
  case Lexer::typeEscape:            return "Escape";
  case Lexer::typeEscapeHex:         return "EscapeHex";
  case Lexer::typeEscapeUnicode:     return "EscapeUnicode";
  case Lexer::typeDate:              return "Date";
  case Lexer::typeDuration:          return "Duration";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Complete Unicode whitespace list.
//
// http://en.wikipedia.org/wiki/Whitespace_character
// Updated 2013-11-18
bool Lexer::is_ws (int c)
{
  return (c == 0x0020 ||   // space Common  Separator, space
          c == 0x0009 ||   // Common  Other, control  HT, Horizontal Tab
          c == 0x000A ||   // Common  Other, control  LF, Line feed
          c == 0x000B ||   // Common  Other, control  VT, Vertical Tab
          c == 0x000C ||   // Common  Other, control  FF, Form feed
          c == 0x000D ||   // Common  Other, control  CR, Carriage return
          c == 0x0085 ||   // Common  Other, control  NEL, Next line
          c == 0x00A0 ||   // no-break space  Common  Separator, space
          c == 0x1680 ||   // ogham space mark  Ogham Separator, space
          c == 0x180E ||   // mongolian vowel separator Mongolian Separator, space
          c == 0x2000 ||   // en quad Common  Separator, space
          c == 0x2001 ||   // em quad Common  Separator, space
          c == 0x2002 ||   // en space  Common  Separator, space
          c == 0x2003 ||   // em space  Common  Separator, space
          c == 0x2004 ||   // three-per-em space  Common  Separator, space
          c == 0x2005 ||   // four-per-em space Common  Separator, space
          c == 0x2006 ||   // six-per-em space  Common  Separator, space
          c == 0x2007 ||   // figure space  Common  Separator, space
          c == 0x2008 ||   // punctuation space Common  Separator, space
          c == 0x2009 ||   // thin space  Common  Separator, space
          c == 0x200A ||   // hair space  Common  Separator, space
          c == 0x2028 ||   // line separator  Common  Separator, line
          c == 0x2029 ||   // paragraph separator Common  Separator, paragraph
          c == 0x202F ||   // narrow no-break space Common  Separator, space
          c == 0x205F ||   // medium mathematical space Common  Separator, space
          c == 0x3000);    // ideographic space Common  Separator, space
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_punct (int c) const
{
  if (c == ',' ||
      c == '.')      // Tab
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_num (int c) const
{
  if ((c >= '0' && c <= '9') ||
      c == '.')
    return true;

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_ident_start (int c) const
{
  return c           &&       // Include null character check.
         ! is_ws (c) &&
         ! is_dec_digit (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_ident (int c) const
{
  return c           &&       // Include null character check.
         ! is_ws (c) &&
         ! is_single_op (c);
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_triple_op (int c0, int c1, int c2) const
{
  return (c0 == 'a' && c1 == 'n' && c2 == 'd') ||
         (c0 == 'x' && c1 == 'o' && c2 == 'r');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_double_op (int c0, int c1) const
{
  return (c0 == '=' && c1 == '=') ||
         (c0 == '!' && c1 == '=') ||
         (c0 == '<' && c1 == '=') ||
         (c0 == '>' && c1 == '=') ||
         (c0 == 'o' && c1 == 'r') ||
         (c0 == '|' && c1 == '|') ||
         (c0 == '&' && c1 == '&') ||
         (c0 == '!' && c1 == '~');
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_single_op (int c) const
{
  return c == '+' ||
         c == '-' ||
         c == '*' ||
         c == '/' ||
         c == '(' ||
         c == ')' ||
         c == '<' ||
         c == '>' ||
         c == '^' ||
         c == '!' ||
         c == '%' ||
         c == '=' ||
         c == '~';
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_dec_digit (int c) const
{
  return c >= '0' && c <= '9';
}

////////////////////////////////////////////////////////////////////////////////
bool Lexer::is_hex_digit (int c) const
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::decode_escape (int c) const
{
  switch (c)
  {
  case 'b':  return 0x08;
  case 'f':  return 0x0C;
  case 'n':  return 0x0A;
  case 'r':  return 0x0D;
  case 't':  return 0x09;
  case 'v':  return 0x0B;
  case '\'': return 0x27;
  case '"':  return 0x22;
  case '\\': return 0x5C;
  default:   return c;
  }
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hex_to_int (int c) const
{
       if (c >= '0' && c <= '9') return (c - '0');
  else if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
  else                           return (c - 'A' + 10);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hex_to_int (int c0, int c1) const
{
  return (hex_to_int (c0) << 4) + hex_to_int (c1);
}

////////////////////////////////////////////////////////////////////////////////
int Lexer::hex_to_int (int c0, int c1, int c2, int c3) const
{
  return (hex_to_int (c0) << 12) +
         (hex_to_int (c1) << 8)  +
         (hex_to_int (c2) << 4)  +
          hex_to_int (c3);
}

////////////////////////////////////////////////////////////////////////////////
void Lexer::shift ()
{
  _n0 = _n1;
  _n1 = _n2;
  _n2 = _n3;
  _n3 = utf8_next_char (_input, _i);

  //std::cout << "# shift [" << (char) _n0 << (char) _n1 << (char) _n2 << (char) _n3 << "]\n";
}

////////////////////////////////////////////////////////////////////////////////