"""
PlatformIO pre-build script: append C++-only size flags without leaking them
into C compilation units, which only creates warning noise on Windows builds.
"""


CPP_ONLY_FLAGS = [
    "-fno-rtti",
    "-fno-threadsafe-statics",
    "-fno-use-cxa-atexit",
]


def apply_cpp_only_flags(env):
    env.Append(CXXFLAGS=CPP_ONLY_FLAGS)
    print(f"Applied C++-only size flags: {' '.join(CPP_ONLY_FLAGS)}")


Import("env")
apply_cpp_only_flags(env)
