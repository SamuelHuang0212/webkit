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

#ifndef MediaSelectionGroupAVFObjC_h
#define MediaSelectionGroupAVFObjC_h

#if ENABLE(VIDEO_TRACK)

#include "Timer.h"
#include <wtf/HashMap.h>
#include <wtf/IteratorRange.h>
#include <wtf/OwnPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/RetainPtr.h>

OBJC_CLASS AVPlayerItem;
OBJC_CLASS AVMediaSelectionGroup;
OBJC_CLASS AVMediaSelectionOption;

namespace WebCore {

class MediaSelectionGroupAVFObjC;

class MediaSelectionOptionAVFObjC : public RefCounted<MediaSelectionOptionAVFObjC> {
public:
    static PassRefPtr<MediaSelectionOptionAVFObjC> create(MediaSelectionGroupAVFObjC&, AVMediaSelectionOption *);

    void setSelected(bool);
    bool selected() const;

    int index() const;

    AVMediaSelectionOption *avMediaSelectionOption() const { return m_mediaSelectionOption.get(); }

private:
    friend class MediaSelectionGroupAVFObjC;
    MediaSelectionOptionAVFObjC(MediaSelectionGroupAVFObjC&, AVMediaSelectionOption *);

    void clearGroup() { m_group = nullptr; }

    MediaSelectionGroupAVFObjC* m_group;
    RetainPtr<AVMediaSelectionOption> m_mediaSelectionOption;
};

class MediaSelectionGroupAVFObjC : public RefCounted<MediaSelectionGroupAVFObjC> {
public:
    static PassRefPtr<MediaSelectionGroupAVFObjC> create(AVPlayerItem*, AVMediaSelectionGroup*);
    ~MediaSelectionGroupAVFObjC();

    void setSelectedOption(MediaSelectionOptionAVFObjC*);
    MediaSelectionOptionAVFObjC* selectedOption() const { return m_selectedOption; }

    void updateOptions();

    typedef HashMap<AVMediaSelectionOption*, RefPtr<MediaSelectionOptionAVFObjC>> OptionContainer;
    WTF::IteratorRange<OptionContainer::iterator::Values> options() { return m_options.values(); }

    AVMediaSelectionGroup *avMediaSelectionGroup() const { return m_mediaSelectionGroup.get(); }

private:
    MediaSelectionGroupAVFObjC(AVPlayerItem*, AVMediaSelectionGroup*);

    void selectionTimerFired();

    RetainPtr<AVPlayerItem> m_playerItem;
    RetainPtr<AVMediaSelectionGroup> m_mediaSelectionGroup;
    OptionContainer m_options;
    MediaSelectionOptionAVFObjC* m_selectedOption { nullptr };
    Timer m_selectionTimer;
    bool m_shouldSelectOptionAutomatically { true };
};

}

#endif // ENABLE(VIDEO_TRACK)

#endif // MediaSelectionGroupAVFObjC_h
