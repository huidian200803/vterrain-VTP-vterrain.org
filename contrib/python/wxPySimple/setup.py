from distutils.core import setup, Extension

VTP_INSTALL = '/opt/vterrain'
WXPY_DIR = 'usr/local/lib/wxPython-unicode-2.8.9.1'

module1 = Extension('vtpSimple',
                    include_dirs=[VTP_INSTALL + '/include',
                                  '../../../TerrainSDK/',
                                  WXPY_DIR + '/lib/wx/include/mac-unicode-debug-2.8',
                                  WXPY_DIR + '/include/wx-2.8',
                                  '/usr/local/include',
                                  '/sw/include'
                                  ],
                    library_dirs=[VTP_INSTALL + '/lib',
                                  '/usr/local/lib',
                                  '/sw/lib'],
                    libraries=['vtosg'],
                                                      
                    extra_link_args=['-arch', 'i386'], 
                    # #extra_compile_args=['-arch',  'i386'],

                    extra_compile_args=['-arch','i386',
                                        '-D__DARWIN_OSX__',
                                        '-fPIC'] + '-DVTUNIX -DSUPPORT_QUIKGRID=0 -DSUPPORT_NETCDF=1 -DSUPPORT_HTTP=1 -DSUPPORT_BZIP2=1 -DSUPPORT_SQUISH=0 -DUNIX -DVTLIB_OSG=1 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -D__WXDEBUG__ -D__WXMAC__'.split(' '),
                    sources = ['vtpSimple.cpp'])

setup (name = 'vtpSimple',
       version = '0.1',
       description = 'This is a demo of vtpSimple',
       ext_modules = [module1])
