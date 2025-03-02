/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#ifndef NFA_h
#define NFA_h

#if ENABLE(CONTENT_EXTENSIONS)

#include "ContentExtensionsDebugging.h"
#include "NFANode.h"
#include <wtf/Vector.h>

namespace WebCore {

namespace ContentExtensions {

class NFAToDFA;

// The NFA provides a way to build a NFA graph with characters or epsilon as transitions.
// The nodes are accessed through an identifier.
class NFA {
public:
    WEBCORE_EXPORT NFA();
    unsigned root() const { return m_root; }
    unsigned createNode();

    void addTransition(unsigned from, unsigned to, char character);
    void addEpsilonTransition(unsigned from, unsigned to);
    void addTransitionsOnAnyCharacter(unsigned from, unsigned to);
    void setFinal(unsigned node, uint64_t ruleId);

    unsigned graphSize() const;
    void restoreToGraphSize(unsigned);

#if CONTENT_EXTENSIONS_STATE_MACHINE_DEBUGGING
    void addRuleId(unsigned node, uint64_t ruleId);

    void debugPrintDot() const;
#else
    void addRuleId(unsigned, uint64_t) { }
#endif

private:
    friend class NFAToDFA;

    static const unsigned epsilonTransitionCharacter = 256;

    Vector<NFANode> m_nodes;
    unsigned m_root;
};

}

} // namespace WebCore

#endif // ENABLE(CONTENT_EXTENSIONS)

#endif // NFA_h
