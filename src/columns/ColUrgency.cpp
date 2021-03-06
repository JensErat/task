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
#include <ColUrgency.h>
#include <text.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
ColumnUrgency::ColumnUrgency ()
{
  _name  = "urgency";
  _type  = "numeric";
  _style = "real";
  _label = STRING_COLUMN_LABEL_URGENCY;

  _styles.push_back ("real");
  _styles.push_back ("integer");

  _examples.push_back ("4.6");
  _examples.push_back ("4");
}

////////////////////////////////////////////////////////////////////////////////
ColumnUrgency::~ColumnUrgency ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Set the minimum and maximum widths for the value.
void ColumnUrgency::measure (Task& task, unsigned int& minimum, unsigned int& maximum)
{
  if (_style == "default" ||
      _style == "real")
  {
    minimum = maximum = format (task.urgency (), 4, 3).length ();
  }
  else if (_style == "integer")
  {
    minimum = maximum = format ((int)task.urgency ()).length ();
  }

  else
    throw format (STRING_COLUMN_BAD_FORMAT, _name, _style);
}

////////////////////////////////////////////////////////////////////////////////
void ColumnUrgency::render (
  std::vector <std::string>& lines,
  Task& task,
  int width,
  Color& color)
{
  if (_style == "default" ||
      _style == "real")
  {
    lines.push_back (
      color.colorize (
        rightJustify (
          format (task.urgency (), 4, 3), width)));
  }
  else if (_style == "integer")
  {
    lines.push_back (
      color.colorize (
        rightJustify (
          format ((int)task.urgency ()), width)));
  }
}

////////////////////////////////////////////////////////////////////////////////
