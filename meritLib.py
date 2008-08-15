# $Header$
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = 'merit')

    env.Tool('addLibrary', library = env['rootLibs'])

def exists(env):
    return 1;
