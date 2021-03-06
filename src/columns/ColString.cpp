////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
#include <Context.h>
#include <ColString.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnString::ColumnString ()
{
  _name  = "string";
  _type  = "string";
  _style = "left";
  _label = "";

  _styles.push_back ("left");
  _styles.push_back ("right");
  _styles.push_back ("left_fixed");
  _styles.push_back ("right_fixed");

  _styles.push_back ("Hello (wrapped)           ");
  _styles.push_back ("           Hello (wrapped)");
  _styles.push_back ("Hello (no-wrap)           ");
  _styles.push_back ("           Hello (no-wrap)");

  _hyphenate = context.config.getBoolean ("hyphenate");
}

////////////////////////////////////////////////////////////////////////////////
ColumnString::~ColumnString ()
{
}

////////////////////////////////////////////////////////////////////////////////
// ColumnString is unique - it copies the report name into the label.  This is
// a kludgy reuse of an otherwise unused member.
void ColumnString::setReport (const std::string& value)
{
  _report = _label = value;
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
//
void ColumnString::measure (const std::string& value, unsigned int& minimum, unsigned int& maximum)
{
  minimum = maximum = 0;

  if (_style == "left"  ||
      _style == "right" ||
      _style == "default")
  {
    std::string stripped = Color::strip (value);
    maximum = longestLine (stripped);
    minimum = longestWord (stripped);
  }
  else if (_style == "left_fixed" ||
           _style == "right_fixed")
    minimum = maximum = strippedLength (value);
  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnString::render (
  std::vector <std::string>& lines,
  const std::string& value,
  int width,
  Color& color)
{
  if (_style == "default" || _style == "left")
  {
    std::vector <std::string> raw;
    wrapText (raw, value, width, _hyphenate);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (leftJustify (*i, width)));
  }
  else if (_style == "right")
  {
    std::vector <std::string> raw;
    wrapText (raw, value, width, _hyphenate);

    std::vector <std::string>::iterator i;
    for (i = raw.begin (); i != raw.end (); ++i)
      lines.push_back (color.colorize (rightJustify (*i, width)));
  }
  else if (_style == "left_fixed")
  {
    lines.push_back (leftJustify (value, width));
  }
  else if (_style == "right_fixed")
  {
    lines.push_back (rightJustify (value, width));
  }
}

////////////////////////////////////////////////////////////////////////////////
