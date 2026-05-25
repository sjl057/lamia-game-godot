#!/bin/bash

# windows_debug_x86_32_console.exe
# windows_debug_x86_32.exe
# windows_debug_x86_64_console.exe
# windows_debug_x86_64.exe
# windows_debug_arm64_console.exe
# windows_debug_arm64.exe
# windows_release_x86_32_console.exe
# windows_release_x86_32.exe
# windows_release_x86_64_console.exe
# windows_release_x86_64.exe
# windows_release_arm64_console.exe
# windows_release_arm64.exe

scons platform=windows target=template_debug arch=x86_32
scons platform=windows target=template_release arch=x86_32
scons platform=windows target=template_debug arch=x86_64
scons platform=windows target=template_release arch=x86_64
scons platform=windows target=template_debug arch=arm64
scons platform=windows target=template_release arch=arm64

# linux_debug.arm32
# linux_debug.arm64
# linux_debug.x86_32
# linux_debug.x86_64
# linux_release.arm32
# linux_release.arm64
# linux_release.x86_32
# linux_release.x86_64

scons platform=linuxbsd target=template_release arch=x86_32
scons platform=linuxbsd target=template_debug arch=x86_32
scons platform=linuxbsd target=template_release arch=x86_64
scons platform=linuxbsd target=template_debug arch=x86_64
scons platform=linuxbsd target=template_release arch=arm64
scons platform=linuxbsd target=template_debug arch=arm64

