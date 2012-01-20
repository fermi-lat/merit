# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/merit/SConscript,v 1.12 2012/01/04 19:38:41 jrb Exp $ 
# Authors: T.Burnett <tburnett@u.washington.edu>
# Version: merit-06-37-02
import os
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='merit', toBuild='component')
merit = libEnv.ComponentLibrary('merit', listFiles(['src/*.cxx',
						    'src/analysis/*.cxx',
						    'src/meritAlg/*.cxx']))

if baseEnv['PLATFORM'] == 'win32':
	libEnv.AppendUnique(CPPDEFINES = '__i386')

progEnv.Tool('meritLib')
progEnv.Tool('registerTargets', package='merit', libraryCxts=[[merit,libEnv]],
             includes = listFiles(['merit/*.h']))




