#! /usr/bin/env perl
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

use strict;
use warnings;
use Test::More tests => 6;

# Ensure environment has no influence.
delete $ENV{'TASKDATA'};
delete $ENV{'TASKRC'};

use File::Basename;
my $ut = basename ($0);
my $rc = $ut . '.rc';

# Create the rc file.
if (open my $fh, '>', $rc)
{
  print $fh "data.location=.\n";
  close $fh;
}

# Bug 851: Filtering by due dates with ordinal and d/wks/etc. doesn't work
qx{../src/task rc:$rc add yesterday due:-2days 2>&1};
qx{../src/task rc:$rc add tomorrow due:2days 2>&1};
my $output = qx{../src/task rc:$rc ls 2>&1};
like ($output, qr/yesterday/, "$ut: yesterday - task added");
like ($output, qr/tomorrow/, "$ut: tomorrow - task added");

$output = qx{../src/task rc:$rc list due.before:now+1d 2>&1};
like ($output, qr/yesterday/, "$ut: yesterday - found before:1d");
unlike ($output, qr/tomorrow/, "$ut: tomorrow - not found before:1d");

$output = qx{../src/task rc:$rc list due.after:now+1d 2>&1};
unlike ($output, qr/yesterday/, "$ut: yesterday - not found after:1d");
like ($output, qr/tomorrow/, "$ut: tomorrow - found after:1d");

# Cleanup.
unlink qw(pending.data completed.data undo.data backlog.data), $rc;
exit 0;

