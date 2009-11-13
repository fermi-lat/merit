# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/merit/SConscript,v 1.3 2009/01/23 00:07:48 ecephas Exp $ 
# Authors: T.Burnett <tburnett@u.washington.edu>
# Version: merit-06-36-03
import os
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('meritLib', depsOnly = 1)
merit = libEnv.SharedLibrary('merit', listFiles(['src/analysis/*.cxx']) + listFiles(['src/meritAlg/*.cxx']) + listFiles(['src/Dll/*.cxx']))

if baseEnv['PLATFORM'] == 'win32':
	libEnv.AppendUnique(CPPDEFINES = '__i386')

progEnv.Tool('meritLib')
progEnv.Tool('registerTargets', package='merit', libraryCxts=[[merit,libEnv]],
             includes = listFiles(['merit/*.h']))




