#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CXXFLAGS=["-fexceptions"])
env.Append(CPPPATH=["src/", "./", "glucose/"])
sources = Glob("src/*.cpp") + Glob("src/mapf/*.cpp") + Glob("src/nodes/*.cpp")
env.Append(LIBPATH=["glucose/build/"])
env.Append(LIBS=["libglucose", "z"])

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "viewer/bin/libmapfsat.{}.{}.framework/libmapfsat.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "viewer/bin/libmapfsat.{}.{}.simulator.a".format(env["platform"], env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "viewer/bin/libmapfsat.{}.{}.a".format(env["platform"], env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "viewer/bin/libmapfsat{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
