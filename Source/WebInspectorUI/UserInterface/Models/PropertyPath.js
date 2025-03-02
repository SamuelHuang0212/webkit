/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

WebInspector.PropertyPath = function(object, pathComponent, parent, isPrototype)
{
    WebInspector.Object.call(this);

    console.assert(object instanceof WebInspector.RemoteObject || object === null);
    console.assert(!pathComponent || typeof pathComponent === "string");
    console.assert(!parent || parent instanceof WebInspector.PropertyPath);
    console.assert(!parent || pathComponent.length);

    // We allow property pathes with null objects as end-caps only.
    // Disallow appending to a PropertyPath with null objects.
    if (parent && !parent.object)
        throw new Error("Attempted to append to a PropertyPath with null object.");

    this._object = object;
    this._pathComponent = typeof pathComponent === "string" ? pathComponent : null;
    this._parent = parent || null;
    this._isPrototype = isPrototype || false;
};

WebInspector.PropertyPath.SpecialPathComponent = {
    InternalPropertyName: "@internal",
    SymbolPropertyName: "@symbol",
    MapKey: "@mapkey",
    MapValue: "@mapvalue",
    SetIndex: "@setindex",
    EmptyPathComponentForScope: "",
};

WebInspector.PropertyPath.Type = {
    Value: "value",
    Getter: "getter",
    Setter: "setter",
};

WebInspector.PropertyPath.emptyPropertyPathForScope = function(object)
{
    return new WebInspector.PropertyPath(object, WebInspector.PropertyPath.SpecialPathComponent.EmptyPathComponentForScope);
}

WebInspector.PropertyPath.prototype = {
    constructor: WebInspector.PropertyPath,
    __proto__: WebInspector.Object.prototype,

    // Public

    get object()
    {
        return this._object;
    },

    get parent()
    {
        return this._parent;
    },

    get isPrototype()
    {
        return this._isPrototype;
    },

    get rootObject()
    {
        return this._parent ? this._parent.rootObject : this._object;
    },

    get lastNonPrototypeObject()
    {
        if (!this._parent)
            return this._object;

        var p = this._parent;
        while (p) {
            if (!p.isPrototype)
                break;
            if (!p.parent)
                break;
            p = p.parent;
        }

        return p.object;
    },

    get pathComponent()
    {
        return this._pathComponent;
    },

    get fullPath()
    {
        var components = [];
        for (var p = this; p && p.pathComponent; p = p.parent)
            components.push(p.pathComponent);
        components.reverse()
        return components.join("");
    },

    get reducedPath()
    {
        // The display path for a value should not include __proto__.
        // The path for "foo.__proto__.bar.__proto__.x" is better shown as "foo.bar.x".
        // FIXME: We should keep __proto__ if this property was overridden.
        var components = [];

        var p = this;

        // Include trailing __proto__s.
        for (; p && p.isPrototype; p = p.parent)
            components.push(p.pathComponent);

        // Skip other __proto__s.
        for (; p && p.pathComponent; p = p.parent) {
            if (p.isPrototype)
                continue;
            components.push(p.pathComponent);
        }

        components.reverse();
        return components.join("");        
    },

    displayPath(type)
    {
        return type === WebInspector.PropertyPath.Type.Value ? this.reducedPath : this.fullPath;
    },

    isRoot()
    {
        return !this._parent;
    },

    isScope()
    {
        return this._pathComponent === WebInspector.PropertyPath.SpecialPathComponent.EmptyPathComponentForScope;
    },

    isPathComponentImpossible()
    {
        return this._pathComponent && this._pathComponent.startsWith("@");
    },

    isFullPathImpossible()
    {
        if (this.isPathComponentImpossible())
            return true;

        if (this._parent)
            return this._parent.isFullPathImpossible();

        return false;
    },

    appendPropertyName(object, propertyName)
    {
        var isPrototype = propertyName === "__proto__";

        if (this.isScope())
            return new WebInspector.PropertyPath(object, propertyName, this, isPrototype);

        var component = this._canPropertyNameBeDotAccess(propertyName) ? "." + propertyName : "[" + doubleQuotedString(propertyName) + "]";
        return new WebInspector.PropertyPath(object, component, this, isPrototype);
    },

    appendPropertySymbol(object, symbolName)
    {
        var component = WebInspector.PropertyPath.SpecialPathComponent.SymbolPropertyName + (symbolName.length ? "(" + symbolName + ")" : "");
        return new WebInspector.PropertyPath(object, component, this);
    },

    appendInternalPropertyName(object, propertyName)
    {
        var component = WebInspector.PropertyPath.SpecialPathComponent.InternalPropertyName + "[" + propertyName + "]";
        return new WebInspector.PropertyPath(object, component, this);
    },

    appendGetterPropertyName(object, propertyName)
    {
        var component = ".__lookupGetter__(" + doubleQuotedString(propertyName) + ")";
        return new WebInspector.PropertyPath(object, component, this);
    },

    appendSetterPropertyName(object, propertyName)
    {
        var component = ".__lookupSetter__(" + doubleQuotedString(propertyName) + ")";
        return new WebInspector.PropertyPath(object, component, this);
    },

    appendArrayIndex(object, indexString)
    {
        var component = "[" + indexString + "]";
        return new WebInspector.PropertyPath(object, component, this);
    },

    appendMapKey(object)
    {
        var component = WebInspector.PropertyPath.SpecialPathComponent.MapKey;
        return new WebInspector.PropertyPath(object, component, this);
    },

    appendMapValue(object, keyObject)
    {
        console.assert(!keyObject || keyObject instanceof WebInspector.RemoteObject);

        if (keyObject && keyObject.hasValue()) {
            if (keyObject.type === "string") {
                var component = ".get(" + doubleQuotedString(keyObject.description) + ")";
                return new WebInspector.PropertyPath(object, component, this);                
            }

            var component = ".get(" + keyObject.description + ")";
            return new WebInspector.PropertyPath(object, component, this);                
        }
            
        var component = WebInspector.PropertyPath.SpecialPathComponent.MapValue;
        return new WebInspector.PropertyPath(object, component, this);
    },

    appendSetIndex(object)
    {
        var component = WebInspector.PropertyPath.SpecialPathComponent.SetIndex;
        return new WebInspector.PropertyPath(object, component, this);
    },
    
    appendPropertyDescriptor(object, descriptor, type)
    {
        if (descriptor.isInternalProperty)
            return this.appendInternalPropertyName(object, descriptor.name);

        if (type === WebInspector.PropertyPath.Type.Getter)
            return this.appendGetterPropertyName(object, descriptor.name);
        if (type === WebInspector.PropertyPath.Type.Setter)
            return this.appendSetterPropertyName(object, descriptor.name);

        console.assert(type === WebInspector.PropertyPath.Type.Value);

        // FIXME: We don't yet have Symbol descriptors.

        if (this._object.subtype === "array" && !isNaN(parseInt(descriptor.name)))
            return this.appendArrayIndex(object, descriptor.name);

        return this.appendPropertyName(object, descriptor.name)
    },

    // Private

    _canPropertyNameBeDotAccess(propertyName)
    {
        return /^(?![0-9])\w+$/.test(propertyName);
    }
};
