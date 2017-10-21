/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QV4REGEXPOBJECT_H
#define QV4REGEXPOBJECT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qv4runtime_p.h"
#include "qv4engine_p.h"
#include "qv4context_p.h"
#include "qv4functionobject_p.h"
#include "qv4string_p.h"
#include "qv4managed_p.h"
#include "qv4property_p.h"
#include "qv4objectiterator_p.h"
#include "qv4regexp_p.h"

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QScopedPointer>
#include <cstdio>
#include <cassert>

QT_BEGIN_NAMESPACE

namespace QV4 {

namespace Heap {

#define RegExpObjectMembers(class, Member) \
    Member(class, Pointer, RegExp *, value)

DECLARE_HEAP_OBJECT(RegExpObject, Object) {
    DECLARE_MARK_TABLE(RegExpObject);

    void init();
    void init(QV4::RegExp *value);
    void init(const QRegExp &re);
};

#define RegExpCtorMembers(class, Member) \
    Member(class, HeapValue, HeapValue, lastMatch) \
    Member(class, Pointer, String *, lastInput) \
    Member(class, NoMark, int, lastMatchStart) \
    Member(class, NoMark, int, lastMatchEnd)

DECLARE_HEAP_OBJECT(RegExpCtor, FunctionObject) {
    DECLARE_MARK_TABLE(RegExpCtor);

    void init(QV4::ExecutionContext *scope);
    void clearLastMatch();
};

}

struct RegExpObject: Object {
    V4_OBJECT2(RegExpObject, Object)
    Q_MANAGED_TYPE(RegExpObject)
    V4_INTERNALCLASS(RegExpObject)
    V4_PROTOTYPE(regExpPrototype)

    // needs to be compatible with the flags in qv4compileddata_p.h
    enum Flags {
        RegExp_Global     = 0x01,
        RegExp_IgnoreCase = 0x02,
        RegExp_Multiline  = 0x04
    };

    enum {
        Index_LastIndex = 0,
        Index_Source = 1,
        Index_Global = 2,
        Index_IgnoreCase = 3,
        Index_Multiline = 4,
        Index_ArrayIndex = Heap::ArrayObject::LengthPropertyIndex + 1,
        Index_ArrayInput = Index_ArrayIndex + 1
    };

    Heap::RegExp *value() const { return d()->value; }
    bool global() const { return d()->value->global; }

    void initProperties();

    int lastIndex() const {
        Q_ASSERT(Index_LastIndex == internalClass()->find(engine()->id_lastIndex()));
        return propertyData(Index_LastIndex)->toInt32();
    }
    void setLastIndex(int index) {
        Q_ASSERT(Index_LastIndex == internalClass()->find(engine()->id_lastIndex()));
        return setProperty(Index_LastIndex, Primitive::fromInt32(index));
    }

    QRegExp toQRegExp() const;
    QString toString() const;
    QString source() const;
    uint flags() const;
};

struct RegExpCtor: FunctionObject
{
    V4_OBJECT2(RegExpCtor, FunctionObject)

    Value lastMatch() { return d()->lastMatch; }
    Heap::String *lastInput() { return d()->lastInput; }
    int lastMatchStart() { return d()->lastMatchStart; }
    int lastMatchEnd() { return d()->lastMatchEnd; }

    static ReturnedValue callAsConstructor(const Managed *m, CallData *callData);
    static ReturnedValue call(const Managed *that, CallData *callData);
};

struct RegExpPrototype: RegExpObject
{
    void init(ExecutionEngine *engine, Object *ctor);

    static ReturnedValue method_exec(const BuiltinFunction *, CallData *callData);
    static ReturnedValue method_test(const BuiltinFunction *, CallData *callData);
    static ReturnedValue method_toString(const BuiltinFunction *, CallData *callData);
    static ReturnedValue method_compile(const BuiltinFunction *, CallData *callData);

    template <int index>
    static ReturnedValue method_get_lastMatch_n(const BuiltinFunction *, CallData *callData);
    static ReturnedValue method_get_lastParen(const BuiltinFunction *, CallData *callData);
    static ReturnedValue method_get_input(const BuiltinFunction *, CallData *callData);
    static ReturnedValue method_get_leftContext(const BuiltinFunction *, CallData *callData);
    static ReturnedValue method_get_rightContext(const BuiltinFunction *, CallData *callData);

    static ReturnedValue execFirstMatch(const BuiltinFunction *b, CallData *callData);
};

}

QT_END_NAMESPACE

#endif // QMLJS_OBJECTS_H
