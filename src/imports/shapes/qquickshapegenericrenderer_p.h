/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#ifndef QQUICKSHAPEGENERICRENDERER_P_H
#define QQUICKSHAPEGENERICRENDERER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qquickshape_p_p.h"
#include <qsgnode.h>
#include <qsggeometry.h>
#include <qsgmaterial.h>
#include <qsgrendererinterface.h>
#include <QtCore/qrunnable.h>

QT_BEGIN_NAMESPACE

class QQuickShapeGenericNode;
class QQuickShapeGenericStrokeFillNode;
class QQuickShapeFillRunnable;
class QQuickShapeStrokeRunnable;

class QQuickShapeGenericRenderer : public QQuickAbstractPathRenderer
{
public:
    enum Dirty {
        DirtyFillGeom = 0x01,
        DirtyStrokeGeom = 0x02,
        DirtyColor = 0x04,
        DirtyFillGradient = 0x08,
        DirtyList = 0x10 // only for accDirty
    };

    QQuickShapeGenericRenderer(QQuickItem *item)
        : m_item(item),
          m_api(QSGRendererInterface::Unknown),
          m_rootNode(nullptr),
          m_accDirty(0),
          m_asyncCallback(nullptr),
          m_asyncCallbackData(nullptr)
    { }
    ~QQuickShapeGenericRenderer();

    void beginSync(int totalCount) override;
    void setPath(int index, const QQuickPath *path) override;
    void setJSPath(int index, const QQuickShapePathCommands &path) override;
    void setStrokeColor(int index, const QColor &color) override;
    void setStrokeWidth(int index, qreal w) override;
    void setFillColor(int index, const QColor &color) override;
    void setFillRule(int index, QQuickShapePath::FillRule fillRule) override;
    void setJoinStyle(int index, QQuickShapePath::JoinStyle joinStyle, int miterLimit) override;
    void setCapStyle(int index, QQuickShapePath::CapStyle capStyle) override;
    void setStrokeStyle(int index, QQuickShapePath::StrokeStyle strokeStyle,
                        qreal dashOffset, const QVector<qreal> &dashPattern) override;
    void setFillGradient(int index, QQuickShapeGradient *gradient) override;
    void endSync(bool async) override;
    void setAsyncCallback(void (*)(void *), void *) override;
    Flags flags() const override { return SupportsAsync; }

    void updateNode() override;

    void setRootNode(QQuickShapeGenericNode *node);

    struct Color4ub { unsigned char r, g, b, a; };
    typedef QVector<QSGGeometry::ColoredPoint2D> VertexContainerType;
    typedef QVector<quint32> IndexContainerType;

    static void triangulateFill(const QPainterPath &path,
                                const Color4ub &fillColor,
                                VertexContainerType *fillVertices,
                                IndexContainerType *fillIndices,
                                QSGGeometry::Type *indexType,
                                bool supportsElementIndexUint);
    static void triangulateStroke(const QPainterPath &path,
                                  const QPen &pen,
                                  const Color4ub &strokeColor,
                                  VertexContainerType *strokeVertices,
                                  const QSize &clipSize);

private:
    void maybeUpdateAsyncItem();

    struct ShapePathData {
        float strokeWidth;
        QPen pen;
        Color4ub strokeColor;
        Color4ub fillColor;
        Qt::FillRule fillRule;
        QPainterPath path;
        bool fillGradientActive;
        QQuickShapeGradientCache::GradientDesc fillGradient;
        VertexContainerType fillVertices;
        IndexContainerType fillIndices;
        QSGGeometry::Type indexType;
        VertexContainerType strokeVertices;
        int syncDirty;
        int effectiveDirty = 0;
        QQuickShapeFillRunnable *pendingFill = nullptr;
        QQuickShapeStrokeRunnable *pendingStroke = nullptr;
    };

    void updateShadowDataInNode(ShapePathData *d, QQuickShapeGenericStrokeFillNode *n);
    void updateFillNode(ShapePathData *d, QQuickShapeGenericNode *node);
    void updateStrokeNode(ShapePathData *d, QQuickShapeGenericNode *node);

    QQuickItem *m_item;
    QSGRendererInterface::GraphicsApi m_api;
    QQuickShapeGenericNode *m_rootNode;
    QVector<ShapePathData> m_sp;
    int m_accDirty;
    void (*m_asyncCallback)(void *);
    void *m_asyncCallbackData;
};

class QQuickShapeFillRunnable : public QObject, public QRunnable
{
    Q_OBJECT

public:
    void run() override;

    bool orphaned = false;

    // input
    QPainterPath path;
    QQuickShapeGenericRenderer::Color4ub fillColor;
    bool supportsElementIndexUint;

    // output
    QQuickShapeGenericRenderer::VertexContainerType fillVertices;
    QQuickShapeGenericRenderer::IndexContainerType fillIndices;
    QSGGeometry::Type indexType;

Q_SIGNALS:
    void done(QQuickShapeFillRunnable *self);
};

class QQuickShapeStrokeRunnable : public QObject, public QRunnable
{
    Q_OBJECT

public:
    void run() override;

    bool orphaned = false;

    // input
    QPainterPath path;
    QPen pen;
    QQuickShapeGenericRenderer::Color4ub strokeColor;
    QSize clipSize;

    // output
    QQuickShapeGenericRenderer::VertexContainerType strokeVertices;

Q_SIGNALS:
    void done(QQuickShapeStrokeRunnable *self);
};

class QQuickShapeGenericStrokeFillNode : public QSGGeometryNode
{
public:
    QQuickShapeGenericStrokeFillNode(QQuickWindow *window);
    ~QQuickShapeGenericStrokeFillNode();

    enum Material {
        MatSolidColor,
        MatLinearGradient
    };

    void activateMaterial(Material m);

    QQuickWindow *window() const { return m_window; }

    // shadow data for custom materials
    QQuickShapeGradientCache::GradientDesc m_fillGradient;

private:
    QSGGeometry *m_geometry;
    QQuickWindow *m_window;
    QSGMaterial *m_material;
    QScopedPointer<QSGMaterial> m_solidColorMaterial;
    QScopedPointer<QSGMaterial> m_linearGradientMaterial;

    friend class QQuickShapeGenericRenderer;
};

class QQuickShapeGenericNode : public QSGNode
{
public:
    QQuickShapeGenericStrokeFillNode *m_fillNode = nullptr;
    QQuickShapeGenericStrokeFillNode *m_strokeNode = nullptr;
    QQuickShapeGenericNode *m_next = nullptr;
};

class QQuickShapeGenericMaterialFactory
{
public:
    static QSGMaterial *createVertexColor(QQuickWindow *window);
    static QSGMaterial *createLinearGradient(QQuickWindow *window, QQuickShapeGenericStrokeFillNode *node);
};

#ifndef QT_NO_OPENGL

class QQuickShapeLinearGradientShader : public QSGMaterialShader
{
public:
    QQuickShapeLinearGradientShader();

    void initialize() override;
    void updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect) override;
    char const *const *attributeNames() const override;

    static QSGMaterialType type;

private:
    int m_opacityLoc = -1;
    int m_matrixLoc = -1;
    int m_gradStartLoc = -1;
    int m_gradEndLoc = -1;
};

class QQuickShapeLinearGradientMaterial : public QSGMaterial
{
public:
    QQuickShapeLinearGradientMaterial(QQuickShapeGenericStrokeFillNode *node)
        : m_node(node)
    {
        // Passing RequiresFullMatrix is essential in order to prevent the
        // batch renderer from baking in simple, translate-only transforms into
        // the vertex data. The shader will rely on the fact that
        // vertexCoord.xy is the Shape-space coordinate and so no modifications
        // are welcome.
        setFlag(Blending | RequiresFullMatrix);
    }

    QSGMaterialType *type() const override
    {
        return &QQuickShapeLinearGradientShader::type;
    }

    int compare(const QSGMaterial *other) const override;

    QSGMaterialShader *createShader() const override
    {
        return new QQuickShapeLinearGradientShader;
    }

    QQuickShapeGenericStrokeFillNode *node() const { return m_node; }

private:
    QQuickShapeGenericStrokeFillNode *m_node;
};

#endif // QT_NO_OPENGL

QT_END_NAMESPACE

#endif // QQUICKSHAPEGENERICRENDERER_P_H
