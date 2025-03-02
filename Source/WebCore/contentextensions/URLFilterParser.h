/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef URLFilterParser_h
#define URLFilterParser_h

#if ENABLE(CONTENT_EXTENSIONS)

#include <wtf/HashMap.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

namespace ContentExtensions {

class NFA;

struct PrefixTreeEntry;

class WEBCORE_EXPORT URLFilterParser {
public:
    enum ParseStatus {
        Ok,
        MatchesEverything,
        UnclosedGroups,
        CannotMatchAnything,
        NonASCII,
        UnsupportedCharacterClass,
        MisplacedQuantifier,
        BackReference,
        MisplacedStartOfLine,
        WordBoundary,
        AtomCharacter,
        Group,
        Disjunction,
        MisplacedEndOfLine,
        EmptyPattern,
        YarrError,
        InvalidQuantifier,
    };
    static String statusString(ParseStatus);
    explicit URLFilterParser(NFA&);
    ~URLFilterParser();
    ParseStatus addPattern(const String& pattern, bool patternIsCaseSensitive, uint64_t patternId);

private:
    NFA& m_nfa;
    std::unique_ptr<PrefixTreeEntry> m_prefixTreeRoot;
};

} // namespace ContentExtensions

} // namespace WebCore

#endif // ENABLE(CONTENT_EXTENSIONS)

#endif // URLFilterParser_h
