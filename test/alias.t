#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# http://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestAlias(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        # Used to initialize objects that should be re-initialized or
        # re-created for each individual test
        self.t = Task()

        self.t.config("alias.foo", "_projects")
        self.t.config("alias.bar", "foo")
        self.t.config("alias.baz", "bar")
        self.t.config("alias.qux", "baz")

    def test_alias_to_project(self):
        """Access a project via aliases"""

        expected = "ALIAS"
        self.t(("add", "project:{0}".format(expected), "foo"))

        code, out, err = self.t(("_projects",))
        self.assertIn(expected, out,
                      msg="task _projects -> ALIAS")

        code, out, err = self.t(("foo",))
        self.assertIn(expected, out,
                      msg="task foo -> _projects > ALIAS")

        code, out, err = self.t(("bar",))
        self.assertIn(expected, out,
                      msg="task bar -> foo > _projects > ALIAS")

        code, out, err = self.t(("baz",))
        self.assertIn(expected, out,
                      msg="task baz -> bar > foo > _projects > ALIAS")

        code, out, err = self.t(("qux",))
        self.assertIn(expected, out,
                      msg="task qux -> baz > bar > foo > _projects > ALIAS")

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
