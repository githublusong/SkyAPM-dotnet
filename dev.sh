#!/bin/sh

export CORECLR_PROFILER={cf0d821e-299b-5307-a3d8-b283c03916dd}
export CORECLR_ENABLE_PROFILING=1
export CORECLR_PROFILER_PATH=~/SkyAPM-dotnet/src/SkyApm.ClrProfiler/build/SkyApm.ClrProfiler.so
export CORECLR_PROFILER_HOME=~/SkyAPM-dotnet/src/SkyApm.ClrProfiler.Trace/bin/Debug/netstandard2.0

code

# vscode debugger will not load profiler.so (run sample in terminal)
# build SkyApm.Transport.Grpc.Protocol.csproj will not gen protocol code , now copy it from windows (need be solved). 