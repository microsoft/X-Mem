# The MIT License (MIT)
# 
# Copyright (c) 2014 Microsoft
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Author: Mark Gottscho <mgottscho@ucla.edu>

# This is a wrapper to a subscript so that we can put build output separate from src/

import platform

hostos = platform.system()
arch = ARGUMENTS.get('arch', 'x64') # Default arch is x64 (x86-64 a.k.a. AMD64) but lacking AVX extensions

#env = Environment()

SConscript('SConscript', variant_dir = 'build/'+hostos.lower()+'/'+arch+'/release', duplicate = 0, exports = {'MODE':'release'}, arch = arch, hostos = hostos)
