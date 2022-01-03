/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== COPYING ==================
static const unsigned char temp_binary_data_0[] =
"Copyright (c) 2003-2010 Mark Borgerding . All rights reserved.\n"
"\n"
"KISS FFT is provided under:\n"
"\n"
"  SPDX-License-Identifier: BSD-3-Clause\n"
"\n"
"Being under the terms of the BSD 3-clause \"New\" or \"Revised\" License,\n"
"according with:\n"
"\n"
"  LICENSES/BSD-3-Clause\n"
"\n";

const char* COPYING = (const char*) temp_binary_data_0;

//================== COPYING ==================
static const unsigned char temp_binary_data_1[] =
"Copyright 2002-2007 \tXiph.org Foundation\n"
"Copyright 2002-2007 \tJean-Marc Valin\n"
"Copyright 2005-2007\tAnalog Devices Inc.\n"
"Copyright 2005-2007\tCommonwealth Scientific and Industrial Research \n"
"                        Organisation (CSIRO)\n"
"Copyright 1993, 2002, 2006 David Rowe\n"
"Copyright 2003 \t\tEpicGames\n"
"Copyright 1992-1994\tJutta Degener, Carsten Bormann\n"
"\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions\n"
"are met:\n"
"\n"
"- Redistributions of source code must retain the above copyright\n"
"notice, this list of conditions and the following disclaimer.\n"
"\n"
"- Redistributions in binary form must reproduce the above copyright\n"
"notice, this list of conditions and the following disclaimer in the\n"
"documentation and/or other materials provided with the distribution.\n"
"\n"
"- Neither the name of the Xiph.org Foundation nor the names of its\n"
"contributors may be used to endorse or promote products derived from\n"
"this software without specific prior written permission.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
"``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
"LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
"A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR\n"
"CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
"EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
"PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
"NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";

const char* COPYING2 = (const char*) temp_binary_data_1;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x63a1442d:  numBytes = 246; return COPYING;
        case 0x108741a5:  numBytes = 1774; return COPYING2;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "COPYING",
    "COPYING2"
};

const char* originalFilenames[] =
{
    "COPYING",
    "COPYING"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
    {
        if (namedResourceList[i] == resourceNameUTF8)
            return originalFilenames[i];
    }

    return nullptr;
}

}
