# Convert NM output from sysv style to markdown-compatible style
# Output you data using "nm --format=sysv --size-sort your.elf"
cmake_minimum_required(VERSION 3.18)

file(STRINGS ${CMAKE_ARGV3} inLines)
while(inLines)
    list(POP_FRONT inLines line)
    if(line STREQUAL "")
        # skip line
    elseif(line MATCHES "^Name.*Value.*Class.*Type.*Size.*Line.*Section")
        # add markdown header
        list(APPEND outLines "Name | Value | Class | Type | Size | Line | Section\n")
        list(APPEND outLines "---- | ----- | ----- | ---- | ---- | ---- | -------\n")
    else()
        list(APPEND outLines "${line}\n")
    endif()
endwhile()
file(WRITE ${CMAKE_ARGV4} ${outLines})
