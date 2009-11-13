# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/merit/meritLib.py,v 1.2 2009/08/05 23:04:19 jrb Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['merit'])

    env.Tool('EventLib')
    env.Tool('LdfEventLib')
    env.Tool('facilitiesLib')
    env.Tool('AnalysisNtupleLib')
    env.Tool('GuiSvcLib')
    env.Tool('addLibrary', library = env['rootLibs'])
    env.Tool('addLibrary', library = env['clhepLibs'])
    env.Tool('addLibrary', library = env['cppunitLibs'])
    env.Tool('addLibrary', library = env['gaudiLibs'])
    env.Tool('addLibrary', library = env['cfitsioLibs'])
    env.Tool('addLibrary', library = env['rootLibs'])
    env.Tool('addLibrary', library = env['rootLibs'])

def exists(env):
    return 1;
