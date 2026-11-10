# Contributing to Barrett

Thank you for your interest in contributing to Barrett.

## Code of Conduct

Please be polite and patient with the maintainers. The maintainers are often busy with little spare time, or take breaks from Barrett's development. 

## AI Policy

Any contributions created in-part or in-whole by generative AI are strictly forbidden and will be rejected.
This restriction extends to all aspects of a submission, such as code, PR descriptions, or issues.

This restriction *does not* extend to standard algorithmic code generators or analyzers such as:

- Algorithmic auto-completion & suggestions (e.g. IntelliSense)
- Linters & formatters
- Macros, templates, snippets or scripts

## Bug Reporting & Issues

Barrett contains an innumerable amount of bugs that are complex to resolve and debug due to multi-faceted layers of legacy code decay.

Due to the complexity involved, bug reports require extra detail. 
The following guide will assist you to make the most comprehensive bug report possible.

### General Guidelines

Give your issue a concise, descriptive title.

In the description body, please give a description of:

- Your computer's configuration, such as your CPU, and operating system (LINUX USERS: including DE/WM & if on X11 or Wayland).
- The version of Barrett you are currently running.
- The version of ROTT that your WAD, RTL/RTLX or KPF files are sourced from. (e.g. Ludicrous, v1.2, v1.3)
- A brief description of the behaviour or exhibited by the bug
- A brief description of the intended behaviour
- The steps or patterns that lead to the bug being triggered.
- If you were replicate the bug, including any potentially different steps it took to replicate on subsequent attempts.

Where applicable, a *video* or *screenshot* of the issue could be very useful, for instance gameplay footage where the bug is triggered.

### Reporting Fatal Crashes

In addition to the prior guidelines, these additional steps should be taken for fatal crashes.

Please *screenshot* or *paste* the **entire** stack trace output from the command line at the bottom of your issue.

For release builds, this will look something like:

```
OH NO OH NO ROTT CRASHED!
Here is where:
Stack dump:
{
    /lib64/libc.so.6(+0x19fb0) [0x7fd919220fb0]
    ./barrett() [0x552f27]
    ...
}
```

On a debug build with GCC or Clang (not applicable to users who downloaded Barrett from the releases tab), 
it will look something like:

```
==235624==ERROR: LeakSanitizer: detected memory leaks
 
Direct leak of 100 byte(s) in 1 object(s) allocated from:
    #0 0xFFFFFFFFF in __interceptor_malloc ../../.././libsanitizer/asan/asan_malloc_linux.cc:144
    #1 0x12345678 in main /test/asan/leak.c:1
    #2 0xDEADBEEF in __libc_start_main (/lib64/libc.so.6+0x22544)
 
SUMMARY: AddressSanitizer: 100 byte(s) leaked in 1 allocation(s).
```

Please *screenshot* or *paste* the text in any error window that may appear.

### Bugs or Crashes with Loading or Writing Savegames

Please attach the savegame file to the issue.

The file can be found in the userdata/ folder in your Barrett directory.

The file will be named `ROTTGAMx.ROT`, where x is the slot number where the save was written. 
Generally, it will be the most recently modified file in the directory.

For non-fatal bugs, please also describe any odd behaviour exhibited by Barrett upon loading the broken savegame.

## Commit Guidelines

Barrett commits are labelled by scope as such:

`scope: description`

e.g. `fix: asan crashes on new column funcs`

Generally for single file changes, the scope is simply the name of the file, including the .c or .h extension.

Commits are generally preferred to be as short and narrow as possible to allow more granularity with searching or reverting changes.
However, for larger commits, you should use a scope such as `add`, `refactor`, `remove`, or even the modified subsystem, such as `render`.

Try to avoid including out-of-scope changes in a commit where possible, such as updating a variable elsewhere.
If such a change is awkward to commit separately, try to leave a brief footnote in your commit or PR message.

## PR Guidelines

! TODO