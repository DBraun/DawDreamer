#pragma once

#include <string>
#include <vector>

/**
 * Helper class to safely build argc/argv for Faust C API calls.
 * Automatically manages memory and avoids manual strdup/free bugs.
 *
 * Usage:
 *   FaustArgvBuilder args;
 *   args.add("-I");
 *   args.add(pathToFaustLibraries);
 *   someFunction(args.argc(), args.argv());
 */
class FaustArgvBuilder
{
  public:
    FaustArgvBuilder()
    {
        // Reserve space to prevent reallocation which would invalidate c_str() pointers
        storage.reserve(64);
        argv_ptrs.reserve(64);
    }

    /**
     * Add a single argument.
     * The string is copied and its lifetime is managed by this class.
     */
    void add(const std::string& arg)
    {
        storage.push_back(arg);
        // Rebuild argv_ptrs from scratch after each addition to avoid invalidation
        rebuildArgvPointers();
    }

    /**
     * Add multiple arguments from a vector.
     */
    void add(const std::vector<std::string>& args)
    {
        storage.reserve(storage.size() + args.size());
        for (const auto& arg : args)
        {
            storage.push_back(arg);
        }
        // Rebuild argv_ptrs from scratch after adding all
        rebuildArgvPointers();
    }

    /**
     * Get the argument count.
     */
    int argc() const { return static_cast<int>(argv_ptrs.size()); }

    /**
     * Get the argv array (const char**).
     */
    const char** argv() { return argv_ptrs.data(); }

    /**
     * Clear all arguments.
     */
    void clear()
    {
        storage.clear();
        argv_ptrs.clear();
    }

  private:
    /**
     * Rebuild argv_ptrs from storage.
     * Called after any modification to storage to ensure pointers are valid.
     */
    void rebuildArgvPointers()
    {
        argv_ptrs.clear();
        argv_ptrs.reserve(storage.size());
        for (const auto& str : storage)
        {
            argv_ptrs.push_back(str.c_str());
        }
    }

    std::vector<std::string> storage;   // Owns the string data
    std::vector<const char*> argv_ptrs; // Points into storage
};
