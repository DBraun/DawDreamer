We want to ship DawDreamer with universal libs (macOS x86_64 and "Apple Silicon").
So remember to build libfaust separately for each version and then merge with lipo:

`lipo x86_64.libfaust.2.dylib arm64.libfaust.2.dylib -create -output libfaust.a`