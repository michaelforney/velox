# - Try to find LibYAML
# Once done, this will define
#
#  LibYAML_FOUND - system has LibYAML
#  LibYAML_INCLUDE_DIRS - the LibYAML include directories
#  LibYAML_LIBRARIES - link these to use LibYAML

include(LibFindMacros)

find_path(LibYAML_INCLUDE_DIR
    NAMES yaml.h
)

find_library(LibYAML_LIBRARY
    NAMES yaml
)

set(LibYAML_PROCESS_INCLUDES LibYAML_INCLUDE_DIR)
set(LibYAML_PROCESS_LIBS LibYAML_LIBRARY)
libfind_process(LibYAML)

