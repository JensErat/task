#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
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

import json
import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

class Test1452(TestCase):
    def setUp(self):
        self.t = Task()
        self.t(('add', 'task'))
        self.task_uuid = json.loads(self.t(('1', 'export'))[1].strip())['uuid']

    def test_get_task_by_uuid_with_prefix(self):
        """Tries to filter task simply by it's uuid, using uuid: prefix."""

        # Load task
        output = self.t(('uuid:%s' % self.task_uuid, 'export'))[1]

        # Sanity check it is the correct one
        self.assertEqual(json.loads(output.strip())['uuid'], self.task_uuid)

    def test_get_task_by_uuid_without_prefix(self):
        """Tries to filter task simply by it's uuid, without using uuid: prefix."""

        # Load task
        output = self.t((self.task_uuid, 'export'))[1]

        # Sanity check it is the correct one
        self.assertEqual(json.loads(output.strip())['uuid'], self.task_uuid)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
