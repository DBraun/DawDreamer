/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_BASE_FUNCTIONS \
    X (glActiveTexture) \
    X (glBindBuffer) \
    X (glDeleteBuffers) \
    X (glGenBuffers) \
    X (glBufferData) \
    X (glBufferSubData) \
    X (glCreateProgram) \
    X (glDeleteProgram) \
    X (glCreateShader) \
    X (glDeleteShader) \
    X (glShaderSource) \
    X (glCompileShader) \
    X (glAttachShader) \
    X (glLinkProgram) \
    X (glUseProgram) \
    X (glGetShaderiv) \
    X (glGetShaderInfoLog) \
    X (glGetProgramInfoLog) \
    X (glGetProgramiv) \
    X (glGetUniformLocation) \
    X (glGetAttribLocation) \
    X (glVertexAttribPointer) \
    X (glEnableVertexAttribArray) \
    X (glDisableVertexAttribArray) \
    X (glUniform1f) \
    X (glUniform1i) \
    X (glUniform2f) \
    X (glUniform3f) \
    X (glUniform4f) \
    X (glUniform4i) \
    X (glUniform1fv) \
    X (glUniformMatrix2fv) \
    X (glUniformMatrix3fv) \
    X (glUniformMatrix4fv) \
    X (glBindAttribLocation)

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_EXTENSION_FUNCTIONS \
    X (glIsRenderbuffer) \
    X (glBindRenderbuffer) \
    X (glDeleteRenderbuffers) \
    X (glGenRenderbuffers) \
    X (glRenderbufferStorage) \
    X (glGetRenderbufferParameteriv) \
    X (glIsFramebuffer) \
    X (glBindFramebuffer) \
    X (glDeleteFramebuffers) \
    X (glGenFramebuffers) \
    X (glCheckFramebufferStatus) \
    X (glFramebufferTexture2D) \
    X (glFramebufferRenderbuffer) \
    X (glGetFramebufferAttachmentParameteriv)

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_VERTEXBUFFER_FUNCTIONS \
    X (glGenVertexArrays) \
    X (glDeleteVertexArrays) \
    X (glBindVertexArray)

/** This class contains a generated list of OpenGL extension functions, which are either dynamically loaded
    for a specific GL context, or simply call-through to the appropriate OS function where available.

    This class is provided for backwards compatibility. In new code, you should prefer to use
    functions from the juce::gl namespace. By importing all these symbols with
    `using namespace ::juce::gl;`, all GL enumerations and functions will be made available at
    global scope. This may be helpful if you need to write code with C source compatibility, or
    which is compatible with a different extension-loading library.
    All the normal guidance about `using namespace` should still apply - don't do this in a header,
    or at all if you can possibly avoid it!

    @tags{OpenGL}
*/
struct OpenGLExtensionFunctions
{
    /** A more complete set of GL commands can be found in the juce::gl namespace.

        You should use juce::gl::loadFunctions() to load GL functions.
    */
    JUCE_DEPRECATED (static void initialise());

   #if JUCE_WINDOWS && ! DOXYGEN
    typedef char GLchar;
    typedef pointer_sized_int GLsizeiptr;
    typedef pointer_sized_int GLintptr;
   #endif

   #define X(name) static decltype (::juce::gl::name)& name;
    JUCE_GL_BASE_FUNCTIONS
    JUCE_GL_EXTENSION_FUNCTIONS
    JUCE_GL_VERTEXBUFFER_FUNCTIONS
   #undef X
};

enum MissingOpenGLDefinitions
{
   #if JUCE_ANDROID
    JUCE_RGBA_FORMAT                = ::juce::gl::GL_RGBA,
   #else
    JUCE_RGBA_FORMAT                = ::juce::gl::GL_BGRA_EXT,
   #endif
};

} // namespace juce
