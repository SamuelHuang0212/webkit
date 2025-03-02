/*
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *               1999 Waldo Bastian (bastian@kde.org)
 *               2001 Andreas Schlapbach (schlpbch@iam.unibe.ch)
 *               2001-2003 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2002, 2006, 2007, 2008, 2009, 2010, 2013, 2014 Apple Inc. All rights reserved.
 * Copyright (C) 2008 David Smith (catfish.man@gmail.com)
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "CSSSelector.h"

#include "CSSOMUtils.h"
#include "CSSSelectorList.h"
#include "HTMLNames.h"
#include "SelectorPseudoTypeMap.h"
#include <wtf/Assertions.h>
#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>
#include <wtf/text/AtomicStringHash.h>
#include <wtf/text/StringBuilder.h>

namespace WebCore {

using namespace HTMLNames;

struct SameSizeAsCSSSelector {
    unsigned flags;
    void* unionPointer;
};

static_assert(sizeof(CSSSelector) == sizeof(SameSizeAsCSSSelector), "CSSSelector should remain small.");

CSSSelector::CSSSelector(const QualifiedName& tagQName, bool tagIsForNamespaceRule)
    : m_relation(Descendant)
    , m_match(Tag)
    , m_pseudoType(0)
    , m_parsedNth(false)
    , m_isLastInSelectorList(false)
    , m_isLastInTagHistory(true)
    , m_hasRareData(false)
    , m_hasNameWithCase(false)
    , m_isForPage(false)
    , m_tagIsForNamespaceRule(tagIsForNamespaceRule)
    , m_descendantDoubleChildSyntax(false)
    , m_caseInsensitiveAttributeValueMatching(false)
{
    const AtomicString& tagLocalName = tagQName.localName();
    const AtomicString tagLocalNameASCIILowercase = tagLocalName.convertToASCIILowercase();

    if (tagLocalName == tagLocalNameASCIILowercase) {
        m_data.m_tagQName = tagQName.impl();
        m_data.m_tagQName->ref();
    } else {
        m_data.m_nameWithCase = adoptRef(new NameWithCase(tagQName, tagLocalNameASCIILowercase)).leakRef();
        m_hasNameWithCase = true;
    }
}

void CSSSelector::createRareData()
{
    ASSERT(match() != Tag);
    ASSERT(!m_hasNameWithCase);
    if (m_hasRareData)
        return;
    // Move the value to the rare data stucture.
    m_data.m_rareData = RareData::create(adoptRef(m_data.m_value)).leakRef();
    m_hasRareData = true;
}

static unsigned simpleSelectorSpecificityInternal(const CSSSelector& simpleSelector, bool isComputingMaximumSpecificity);

static unsigned selectorSpecificity(const CSSSelector& firstSimpleSelector, bool isComputingMaximumSpecificity)
{
    unsigned total = simpleSelectorSpecificityInternal(firstSimpleSelector, isComputingMaximumSpecificity);

    for (const CSSSelector* selector = firstSimpleSelector.tagHistory(); selector; selector = selector->tagHistory())
        total = CSSSelector::addSpecificities(total, simpleSelectorSpecificityInternal(*selector, isComputingMaximumSpecificity));
    return total;
}

static unsigned maxSpecificity(const CSSSelectorList& selectorList)
{
    unsigned maxSpecificity = 0;
    for (const CSSSelector* subSelector = selectorList.first(); subSelector; subSelector = CSSSelectorList::next(subSelector))
        maxSpecificity = std::max(maxSpecificity, selectorSpecificity(*subSelector, true));
    return maxSpecificity;
}

static unsigned simpleSelectorSpecificityInternal(const CSSSelector& simpleSelector, bool isComputingMaximumSpecificity)
{
    ASSERT_WITH_MESSAGE(!simpleSelector.isForPage(), "At the time of this writing, page selectors are not treated as real selectors that are matched. The value computed here only account for real selectors.");

    switch (simpleSelector.match()) {
    case CSSSelector::Id:
        return static_cast<unsigned>(SelectorSpecificityIncrement::ClassA);

    case CSSSelector::PagePseudoClass:
        break;
    case CSSSelector::PseudoClass:
        if (simpleSelector.pseudoClassType() == CSSSelector::PseudoClassMatches) {
            ASSERT_WITH_MESSAGE(simpleSelector.selectorList() && simpleSelector.selectorList()->first(), "The parser should never generate a valid selector for an empty :matches().");
            if (!isComputingMaximumSpecificity)
                return 0;
            return maxSpecificity(*simpleSelector.selectorList());
        }

        if (simpleSelector.pseudoClassType() == CSSSelector::PseudoClassNot) {
            ASSERT_WITH_MESSAGE(simpleSelector.selectorList() && simpleSelector.selectorList()->first(), "The parser should never generate a valid selector for an empty :not().");
            return maxSpecificity(*simpleSelector.selectorList());
        }
        FALLTHROUGH;
    case CSSSelector::Exact:
    case CSSSelector::Class:
    case CSSSelector::Set:
    case CSSSelector::List:
    case CSSSelector::Hyphen:
    case CSSSelector::Contain:
    case CSSSelector::Begin:
    case CSSSelector::End:
        return static_cast<unsigned>(SelectorSpecificityIncrement::ClassB);
    case CSSSelector::Tag:
        return (simpleSelector.tagQName().localName() != starAtom) ? static_cast<unsigned>(SelectorSpecificityIncrement::ClassC) : 0;
    case CSSSelector::PseudoElement:
        return static_cast<unsigned>(SelectorSpecificityIncrement::ClassC);
    case CSSSelector::Unknown:
        return 0;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

unsigned CSSSelector::simpleSelectorSpecificity() const
{
    return simpleSelectorSpecificityInternal(*this, false);
}

static unsigned staticSpecificityInternal(const CSSSelector& firstSimpleSelector, bool& ok);

static unsigned simpleSelectorFunctionalPseudoClassStaticSpecificity(const CSSSelector& simpleSelector, bool& ok)
{
    if (simpleSelector.match() == CSSSelector::PseudoClass) {
        CSSSelector::PseudoClassType pseudoClassType = simpleSelector.pseudoClassType();
        if (pseudoClassType == CSSSelector::PseudoClassMatches || pseudoClassType == CSSSelector::PseudoClassNthChild || pseudoClassType == CSSSelector::PseudoClassNthLastChild) {
            const CSSSelectorList* selectorList = simpleSelector.selectorList();
            if (!selectorList) {
                ASSERT_WITH_MESSAGE(pseudoClassType != CSSSelector::PseudoClassMatches, ":matches() should never be created without a valid selector list.");
                return 0;
            }

            const CSSSelector& firstSubselector = *selectorList->first();

            unsigned initialSpecificity = staticSpecificityInternal(firstSubselector, ok);
            if (!ok)
                return 0;

            const CSSSelector* subselector = &firstSubselector;
            while ((subselector = CSSSelectorList::next(subselector))) {
                unsigned subSelectorSpecificity = staticSpecificityInternal(*subselector, ok);
                if (initialSpecificity != subSelectorSpecificity)
                    ok = false;
                if (!ok)
                    return 0;
            }
            return initialSpecificity;
        }
    }
    return 0;
}

static unsigned functionalPseudoClassStaticSpecificity(const CSSSelector& firstSimpleSelector, bool& ok)
{
    unsigned total = 0;
    for (const CSSSelector* selector = &firstSimpleSelector; selector; selector = selector->tagHistory()) {
        total = CSSSelector::addSpecificities(total, simpleSelectorFunctionalPseudoClassStaticSpecificity(*selector, ok));
        if (!ok)
            return 0;
    }
    return total;
}

static unsigned staticSpecificityInternal(const CSSSelector& firstSimpleSelector, bool& ok)
{
    unsigned staticSpecificity = selectorSpecificity(firstSimpleSelector, false);
    return CSSSelector::addSpecificities(staticSpecificity, functionalPseudoClassStaticSpecificity(firstSimpleSelector, ok));
}

unsigned CSSSelector::staticSpecificity(bool &ok) const
{
    ok = true;
    return staticSpecificityInternal(*this, ok);
}

unsigned CSSSelector::addSpecificities(unsigned a, unsigned b)
{
    unsigned total = a;

    unsigned newIdValue = (b & idMask);
    if (((total & idMask) + newIdValue) & ~idMask)
        total |= idMask;
    else
        total += newIdValue;

    unsigned newClassValue = (b & classMask);
    if (((total & classMask) + newClassValue) & ~classMask)
        total |= classMask;
    else
        total += newClassValue;

    unsigned newElementValue = (b & elementMask);
    if (((total & elementMask) + newElementValue) & ~elementMask)
        total |= elementMask;
    else
        total += newElementValue;

    return total;
}

unsigned CSSSelector::specificityForPage() const
{
    ASSERT(isForPage());

    // See http://dev.w3.org/csswg/css3-page/#cascading-and-page-context
    unsigned s = 0;

    for (const CSSSelector* component = this; component; component = component->tagHistory()) {
        switch (component->match()) {
        case Tag:
            s += tagQName().localName() == starAtom ? 0 : 4;
            break;
        case PagePseudoClass:
            switch (component->pagePseudoClassType()) {
            case PagePseudoClassFirst:
                s += 2;
                break;
            case PagePseudoClassLeft:
            case PagePseudoClassRight:
                s += 1;
                break;
            }
            break;
        default:
            break;
        }
    }
    return s;
}

PseudoId CSSSelector::pseudoId(PseudoElementType type)
{
    switch (type) {
    case PseudoElementFirstLine:
        return FIRST_LINE;
    case PseudoElementFirstLetter:
        return FIRST_LETTER;
    case PseudoElementSelection:
        return SELECTION;
    case PseudoElementBefore:
        return BEFORE;
    case PseudoElementAfter:
        return AFTER;
    case PseudoElementScrollbar:
        return SCROLLBAR;
    case PseudoElementScrollbarButton:
        return SCROLLBAR_BUTTON;
    case PseudoElementScrollbarCorner:
        return SCROLLBAR_CORNER;
    case PseudoElementScrollbarThumb:
        return SCROLLBAR_THUMB;
    case PseudoElementScrollbarTrack:
        return SCROLLBAR_TRACK;
    case PseudoElementScrollbarTrackPiece:
        return SCROLLBAR_TRACK_PIECE;
    case PseudoElementResizer:
        return RESIZER;
#if ENABLE(VIDEO_TRACK)
    case PseudoElementCue:
#endif
    case PseudoElementUnknown:
    case PseudoElementUserAgentCustom:
    case PseudoElementWebKitCustom:
        return NOPSEUDO;
    }

    ASSERT_NOT_REACHED();
    return NOPSEUDO;
}

CSSSelector::PseudoElementType CSSSelector::parsePseudoElementType(const String& name)
{
    if (name.isNull())
        return PseudoElementUnknown;

    PseudoElementType type = parsePseudoElementString(*name.impl());
    if (type == PseudoElementUnknown) {
        if (name.startsWith("-webkit-"))
            type = PseudoElementWebKitCustom;

        if (name.startsWith("x-"))
            type = PseudoElementUserAgentCustom;
    }
    return type;
}


bool CSSSelector::operator==(const CSSSelector& other) const
{
    const CSSSelector* sel1 = this;
    const CSSSelector* sel2 = &other;

    while (sel1 && sel2) {
        if (sel1->attribute() != sel2->attribute()
            || sel1->relation() != sel2->relation()
            || sel1->match() != sel2->match()
            || sel1->value() != sel2->value()
            || sel1->m_pseudoType != sel2->m_pseudoType
            || sel1->argument() != sel2->argument()) {
            return false;
        }
        if (sel1->match() == Tag) {
            if (sel1->tagQName() != sel2->tagQName())
                return false;
        }
        sel1 = sel1->tagHistory();
        sel2 = sel2->tagHistory();
    }

    if (sel1 || sel2)
        return false;

    return true;
}

static void appendPseudoClassFunctionTail(StringBuilder& str, const CSSSelector* selector)
{
    switch (selector->pseudoClassType()) {
#if ENABLE(CSS_SELECTORS_LEVEL4)
    case CSSSelector::PseudoClassDir:
#endif
    case CSSSelector::PseudoClassLang:
    case CSSSelector::PseudoClassNthChild:
    case CSSSelector::PseudoClassNthLastChild:
    case CSSSelector::PseudoClassNthOfType:
    case CSSSelector::PseudoClassNthLastOfType:
#if ENABLE(CSS_SELECTORS_LEVEL4)
    case CSSSelector::PseudoClassRole:
#endif
        str.append(selector->argument());
        str.append(')');
        break;
    default:
        break;
    }

}

static void appendLangArgumentList(StringBuilder& str, const Vector<AtomicString>& argumentList)
{
    unsigned argumentListSize = argumentList.size();
    for (unsigned i = 0; i < argumentListSize; ++i) {
        str.append('"');
        str.append(argumentList[i]);
        str.append('"');
        if (i != argumentListSize - 1)
            str.appendLiteral(", ");
    }
}

String CSSSelector::selectorText(const String& rightSide) const
{
    StringBuilder str;

    if (match() == CSSSelector::Tag && !m_tagIsForNamespaceRule) {
        if (tagQName().prefix().isNull())
            str.append(tagQName().localName());
        else {
            str.append(tagQName().prefix().string());
            str.append('|');
            str.append(tagQName().localName());
        }
    }

    const CSSSelector* cs = this;
    while (true) {
        if (cs->match() == CSSSelector::Id) {
            str.append('#');
            serializeIdentifier(cs->value(), str);
        } else if (cs->match() == CSSSelector::Class) {
            str.append('.');
            serializeIdentifier(cs->value(), str);
        } else if (cs->match() == CSSSelector::PseudoClass) {
            switch (cs->pseudoClassType()) {
#if ENABLE(FULLSCREEN_API)
            case CSSSelector::PseudoClassAnimatingFullScreenTransition:
                str.appendLiteral(":-webkit-animating-full-screen-transition");
                break;
#endif
            case CSSSelector::PseudoClassAny: {
                str.appendLiteral(":-webkit-any(");
                cs->selectorList()->buildSelectorsText(str);
                str.append(')');
                break;
            }
#if ENABLE(CSS_SELECTORS_LEVEL4)
            case CSSSelector::PseudoClassAnyLink:
                str.appendLiteral(":any-link");
                break;
#endif
            case CSSSelector::PseudoClassAnyLinkDeprecated:
                str.appendLiteral(":-webkit-any-link");
                break;
            case CSSSelector::PseudoClassAutofill:
                str.appendLiteral(":-webkit-autofill");
                break;
            case CSSSelector::PseudoClassDrag:
                str.appendLiteral(":-webkit-drag");
                break;
            case CSSSelector::PseudoClassFullPageMedia:
                str.appendLiteral(":-webkit-full-page-media");
                break;
#if ENABLE(FULLSCREEN_API)
            case CSSSelector::PseudoClassFullScreen:
                str.appendLiteral(":-webkit-full-screen");
                break;
            case CSSSelector::PseudoClassFullScreenAncestor:
                str.appendLiteral(":-webkit-full-screen-ancestor");
                break;
            case CSSSelector::PseudoClassFullScreenDocument:
                str.appendLiteral(":-webkit-full-screen-document");
                break;
#endif
            case CSSSelector::PseudoClassActive:
                str.appendLiteral(":active");
                break;
            case CSSSelector::PseudoClassChecked:
                str.appendLiteral(":checked");
                break;
            case CSSSelector::PseudoClassCornerPresent:
                str.appendLiteral(":corner-present");
                break;
            case CSSSelector::PseudoClassDecrement:
                str.appendLiteral(":decrement");
                break;
            case CSSSelector::PseudoClassDefault:
                str.appendLiteral(":default");
                break;
#if ENABLE(CSS_SELECTORS_LEVEL4)
            case CSSSelector::PseudoClassDir:
                str.appendLiteral(":dir(");
                appendPseudoClassFunctionTail(str, cs);
                break;
#endif
            case CSSSelector::PseudoClassDisabled:
                str.appendLiteral(":disabled");
                break;
            case CSSSelector::PseudoClassDoubleButton:
                str.appendLiteral(":double-button");
                break;
            case CSSSelector::PseudoClassEmpty:
                str.appendLiteral(":empty");
                break;
            case CSSSelector::PseudoClassEnabled:
                str.appendLiteral(":enabled");
                break;
            case CSSSelector::PseudoClassEnd:
                str.appendLiteral(":end");
                break;
            case CSSSelector::PseudoClassFirstChild:
                str.appendLiteral(":first-child");
                break;
            case CSSSelector::PseudoClassFirstOfType:
                str.appendLiteral(":first-of-type");
                break;
            case CSSSelector::PseudoClassFocus:
                str.appendLiteral(":focus");
                break;
#if ENABLE(VIDEO_TRACK)
            case CSSSelector::PseudoClassFuture:
                str.appendLiteral(":future");
                break;
#endif
            case CSSSelector::PseudoClassHorizontal:
                str.appendLiteral(":horizontal");
                break;
            case CSSSelector::PseudoClassHover:
                str.appendLiteral(":hover");
                break;
            case CSSSelector::PseudoClassInRange:
                str.appendLiteral(":in-range");
                break;
            case CSSSelector::PseudoClassIncrement:
                str.appendLiteral(":increment");
                break;
            case CSSSelector::PseudoClassIndeterminate:
                str.appendLiteral(":indeterminate");
                break;
            case CSSSelector::PseudoClassInvalid:
                str.appendLiteral(":invalid");
                break;
            case CSSSelector::PseudoClassLang:
                str.appendLiteral(":lang(");
                ASSERT_WITH_MESSAGE(cs->langArgumentList() && !cs->langArgumentList()->isEmpty(), "An empty :lang() is invalid and should never be generated by the parser.");
                appendLangArgumentList(str, *cs->langArgumentList());
                str.append(')');
                break;
            case CSSSelector::PseudoClassLastChild:
                str.appendLiteral(":last-child");
                break;
            case CSSSelector::PseudoClassLastOfType:
                str.appendLiteral(":last-of-type");
                break;
            case CSSSelector::PseudoClassLink:
                str.appendLiteral(":link");
                break;
            case CSSSelector::PseudoClassNoButton:
                str.appendLiteral(":no-button");
                break;
            case CSSSelector::PseudoClassNot:
                str.appendLiteral(":not(");
                cs->selectorList()->buildSelectorsText(str);
                str.append(')');
                break;
            case CSSSelector::PseudoClassNthChild:
                str.appendLiteral(":nth-child(");
                str.append(cs->argument());
                if (const CSSSelectorList* selectorList = cs->selectorList()) {
                    str.appendLiteral(" of ");
                    selectorList->buildSelectorsText(str);
                }
                str.append(')');
                break;
            case CSSSelector::PseudoClassNthLastChild:
                str.appendLiteral(":nth-last-child(");
                str.append(cs->argument());
                if (const CSSSelectorList* selectorList = cs->selectorList()) {
                    str.appendLiteral(" of ");
                    selectorList->buildSelectorsText(str);
                }
                str.append(')');
                break;
            case CSSSelector::PseudoClassNthLastOfType:
                str.appendLiteral(":nth-last-of-type(");
                appendPseudoClassFunctionTail(str, cs);
                break;
            case CSSSelector::PseudoClassNthOfType:
                str.appendLiteral(":nth-of-type(");
                appendPseudoClassFunctionTail(str, cs);
                break;
            case CSSSelector::PseudoClassOnlyChild:
                str.appendLiteral(":only-child");
                break;
            case CSSSelector::PseudoClassOnlyOfType:
                str.appendLiteral(":only-of-type");
                break;
            case CSSSelector::PseudoClassOptional:
                str.appendLiteral(":optional");
                break;
            case CSSSelector::PseudoClassMatches: {
                str.appendLiteral(":matches(");
                cs->selectorList()->buildSelectorsText(str);
                str.append(')');
                break;
            }
            case CSSSelector::PseudoClassPlaceholderShown:
                str.appendLiteral(":placeholder-shown");
                break;
            case CSSSelector::PseudoClassOutOfRange:
                str.appendLiteral(":out-of-range");
                break;
#if ENABLE(VIDEO_TRACK)
            case CSSSelector::PseudoClassPast:
                str.appendLiteral(":past");
                break;
#endif
            case CSSSelector::PseudoClassReadOnly:
                str.appendLiteral(":read-only");
                break;
            case CSSSelector::PseudoClassReadWrite:
                str.appendLiteral(":read-write");
                break;
            case CSSSelector::PseudoClassRequired:
                str.appendLiteral(":required");
                break;
#if ENABLE(CSS_SELECTORS_LEVEL4)
            case CSSSelector::PseudoClassRole:
                str.appendLiteral(":role(");
                appendPseudoClassFunctionTail(str, cs);
                break;
#endif
            case CSSSelector::PseudoClassRoot:
                str.appendLiteral(":root");
                break;
            case CSSSelector::PseudoClassScope:
                str.appendLiteral(":scope");
                break;
            case CSSSelector::PseudoClassSingleButton:
                str.appendLiteral(":single-button");
                break;
            case CSSSelector::PseudoClassStart:
                str.appendLiteral(":start");
                break;
            case CSSSelector::PseudoClassTarget:
                str.appendLiteral(":target");
                break;
            case CSSSelector::PseudoClassValid:
                str.appendLiteral(":valid");
                break;
            case CSSSelector::PseudoClassVertical:
                str.appendLiteral(":vertical");
                break;
            case CSSSelector::PseudoClassVisited:
                str.appendLiteral(":visited");
                break;
            case CSSSelector::PseudoClassWindowInactive:
                str.appendLiteral(":window-inactive");
                break;
            case CSSSelector::PseudoClassUnknown:
                ASSERT_NOT_REACHED();
            }
        } else if (cs->match() == CSSSelector::PseudoElement) {
            str.appendLiteral("::");
            str.append(cs->value());
        } else if (cs->isAttributeSelector()) {
            str.append('[');
            const AtomicString& prefix = cs->attribute().prefix();
            if (!prefix.isNull()) {
                str.append(prefix);
                str.append('|');
            }
            str.append(cs->attribute().localName());
            switch (cs->match()) {
                case CSSSelector::Exact:
                    str.append('=');
                    break;
                case CSSSelector::Set:
                    // set has no operator or value, just the attrName
                    str.append(']');
                    break;
                case CSSSelector::List:
                    str.appendLiteral("~=");
                    break;
                case CSSSelector::Hyphen:
                    str.appendLiteral("|=");
                    break;
                case CSSSelector::Begin:
                    str.appendLiteral("^=");
                    break;
                case CSSSelector::End:
                    str.appendLiteral("$=");
                    break;
                case CSSSelector::Contain:
                    str.appendLiteral("*=");
                    break;
                default:
                    break;
            }
            if (cs->match() != CSSSelector::Set) {
                serializeString(cs->value(), str);
                if (cs->attributeValueMatchingIsCaseInsensitive())
                    str.appendLiteral(" i]");
                else
                    str.append(']');
            }
        } else if (cs->match() == CSSSelector::PagePseudoClass) {
            switch (cs->pagePseudoClassType()) {
            case PagePseudoClassFirst:
                str.appendLiteral(":first");
                break;
            case PagePseudoClassLeft:
                str.appendLiteral(":left");
                break;
            case PagePseudoClassRight:
                str.appendLiteral(":right");
                break;
            }
        }

        if (cs->relation() != CSSSelector::SubSelector || !cs->tagHistory())
            break;
        cs = cs->tagHistory();
    }

    if (const CSSSelector* tagHistory = cs->tagHistory()) {
        switch (cs->relation()) {
        case CSSSelector::Descendant:
            if (cs->m_descendantDoubleChildSyntax)
                return tagHistory->selectorText(" >> " + str.toString() + rightSide);
            return tagHistory->selectorText(" " + str.toString() + rightSide);
        case CSSSelector::Child:
            return tagHistory->selectorText(" > " + str.toString() + rightSide);
        case CSSSelector::DirectAdjacent:
            return tagHistory->selectorText(" + " + str.toString() + rightSide);
        case CSSSelector::IndirectAdjacent:
            return tagHistory->selectorText(" ~ " + str.toString() + rightSide);
        case CSSSelector::SubSelector:
            ASSERT_NOT_REACHED();
#if ASSERT_DISABLED
            FALLTHROUGH;
#endif
        case CSSSelector::ShadowDescendant:
            return tagHistory->selectorText(str.toString() + rightSide);
        }
    }
    return str.toString() + rightSide;
}

void CSSSelector::setAttribute(const QualifiedName& value, bool isCaseInsensitive)
{
    createRareData();
    m_data.m_rareData->m_attribute = value;
    m_data.m_rareData->m_attributeCanonicalLocalName = isCaseInsensitive ? value.localName().convertToASCIILowercase() : value.localName();
}

void CSSSelector::setArgument(const AtomicString& value)
{
    createRareData();
    m_data.m_rareData->m_argument = value;
}

void CSSSelector::setLangArgumentList(std::unique_ptr<Vector<AtomicString>> argumentList)
{
    createRareData();
    m_data.m_rareData->m_langArgumentList = WTF::move(argumentList);
}

void CSSSelector::setSelectorList(std::unique_ptr<CSSSelectorList> selectorList)
{
    createRareData();
    m_data.m_rareData->m_selectorList = WTF::move(selectorList);
}

bool CSSSelector::parseNth() const
{
    if (!m_hasRareData)
        return false;
    if (m_parsedNth)
        return true;
    m_parsedNth = m_data.m_rareData->parseNth();
    return m_parsedNth;
}

bool CSSSelector::matchNth(int count) const
{
    ASSERT(m_hasRareData);
    return m_data.m_rareData->matchNth(count);
}

int CSSSelector::nthA() const
{
    ASSERT(m_hasRareData);
    ASSERT(m_parsedNth);
    return m_data.m_rareData->m_a;
}

int CSSSelector::nthB() const
{
    ASSERT(m_hasRareData);
    ASSERT(m_parsedNth);
    return m_data.m_rareData->m_b;
}

CSSSelector::RareData::RareData(PassRefPtr<AtomicStringImpl> value)
    : m_value(value.leakRef())
    , m_a(0)
    , m_b(0)
    , m_attribute(anyQName())
    , m_argument(nullAtom)
{
}

CSSSelector::RareData::~RareData()
{
    if (m_value)
        m_value->deref();
}

// a helper function for parsing nth-arguments
bool CSSSelector::RareData::parseNth()
{
    String argument = m_argument.lower();

    if (argument.isEmpty())
        return false;

    m_a = 0;
    m_b = 0;
    if (argument == "odd") {
        m_a = 2;
        m_b = 1;
    } else if (argument == "even") {
        m_a = 2;
        m_b = 0;
    } else {
        size_t n = argument.find('n');
        if (n != notFound) {
            if (argument[0] == '-') {
                if (n == 1)
                    m_a = -1; // -n == -1n
                else {
                    bool ok;
                    m_a = argument.substringSharingImpl(0, n).toIntStrict(&ok);
                    if (!ok)
                        return false;
                }
            } else if (!n)
                m_a = 1; // n == 1n
            else {
                bool ok;
                m_a = argument.substringSharingImpl(0, n).toIntStrict(&ok);
                if (!ok)
                    return false;
            }

            size_t p = argument.find('+', n);
            if (p != notFound) {
                bool ok;
                m_b = argument.substringSharingImpl(p + 1, argument.length() - p - 1).toIntStrict(&ok);
                if (!ok)
                    return false;
            } else {
                p = argument.find('-', n);
                if (p != notFound) {
                    bool ok;
                    m_b = -argument.substringSharingImpl(p + 1, argument.length() - p - 1).toIntStrict(&ok);
                    if (!ok)
                        return false;
                }
            }
        } else {
            bool ok;
            m_b = argument.toIntStrict(&ok);
            if (!ok)
                return false;
        }
    }
    return true;
}

// a helper function for checking nth-arguments
bool CSSSelector::RareData::matchNth(int count)
{
    if (!m_a)
        return count == m_b;
    else if (m_a > 0) {
        if (count < m_b)
            return false;
        return (count - m_b) % m_a == 0;
    } else {
        if (count > m_b)
            return false;
        return (m_b - count) % (-m_a) == 0;
    }
}

} // namespace WebCore
