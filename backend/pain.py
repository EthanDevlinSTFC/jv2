# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2022 E. Devlin and T. Youngs

import os 

import lxml

path = os.path.abspath(lxml.__file__)
print(path)
print(os.listdir(path))

try:
  import lxml.etree
except:
  print("WTRAFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")