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
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <Context.h>
#include <Command.h>
#include <CmdAttributes.h>
#include <i18n.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdZshAttributes::CmdZshAttributes ()
{
  _keyword     = "_zshattributes";
  _usage       = "task          _zshattributes";
  _description = STRING_CMD_ZSHATTS_USAGE;
  _read_only   = true;
  _displays_id = false;
}

////////////////////////////////////////////////////////////////////////////////
int CmdZshAttributes::execute (std::string& output)
{
  // Get a list of all columns.
  std::vector <std::string> columns = context.getColumns ();

  // Sort alphabetically.
  std::sort (columns.begin (), columns.end ());

  std::stringstream out;
  std::vector <std::string>::iterator c;
  for (c = columns.begin (); c != columns.end (); ++c)
    out << *c << ":" << *c << "\n";

  output = out.str ();
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
