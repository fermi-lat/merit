# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/merit/SConscript,v 1.6 2010/06/11 00:46:46 jrb Exp $ 
# Authors: T.Burnett <tburnett@u.washington.edu>
# Version: merit-06-36-05
import os
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package='merit', toBuild='component')
merit = libEnv.SharedLibrary('merit', listFiles(['src/analysis/*.cxx',
						 'src/meritAlg/*.cxx',
						 'src/Dll/*.cxx']))

if baseEnv['PLATFORM'] == 'win32':
	libEnv.AppendUnique(CPPDEFINES = '__i386')

progEnv.Tool('meritLib')
progEnv.Tool('registerTargets', package='merit', libraryCxts=[[merit,libEnv]],
             includes = listFiles(['merit/*.h']))




