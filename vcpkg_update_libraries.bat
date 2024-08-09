@echo off
title Update libraries to latest
color 10
vcpkg upgrade --no-dry-run
pause