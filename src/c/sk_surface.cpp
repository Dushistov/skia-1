/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

#include "sk_canvas.h"
#include "sk_data.h"
#include "sk_image.h"
#include "sk_paint.h"
#include "sk_path.h"
#include "sk_surface.h"
#include "sk_types_priv.h"

const struct {
    sk_colortype_t  fC;
    SkColorType     fSK;
} gColorTypeMap[] = {
    { UNKNOWN_SK_COLORTYPE,     kUnknown_SkColorType    },
    { RGBA_8888_SK_COLORTYPE,   kRGBA_8888_SkColorType  },
    { BGRA_8888_SK_COLORTYPE,   kBGRA_8888_SkColorType  },
    { ALPHA_8_SK_COLORTYPE,     kAlpha_8_SkColorType    },
};

const struct {
    sk_alphatype_t  fC;
    SkAlphaType     fSK;
} gAlphaTypeMap[] = {
    { OPAQUE_SK_ALPHATYPE,      kOpaque_SkAlphaType     },
    { PREMUL_SK_ALPHATYPE,      kPremul_SkAlphaType     },
    { UNPREMUL_SK_ALPHATYPE,    kUnpremul_SkAlphaType   },
};

static bool from_c_colortype(sk_colortype_t cCT, SkColorType* skCT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypeMap); ++i) {
        if (gColorTypeMap[i].fC == cCT) {
            if (skCT) {
                *skCT = gColorTypeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

static bool to_c_colortype(SkColorType skCT, sk_colortype_t* cCT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gColorTypeMap); ++i) {
        if (gColorTypeMap[i].fSK == skCT) {
            if (cCT) {
                *cCT = gColorTypeMap[i].fC;
            }
            return true;
        }
    }
    return false;
}

static bool from_c_alphatype(sk_alphatype_t cAT, SkAlphaType* skAT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAlphaTypeMap); ++i) {
        if (gAlphaTypeMap[i].fC == cAT) {
            if (skAT) {
                *skAT = gAlphaTypeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

static bool to_c_alphatype(SkAlphaType skAT, sk_alphatype_t &cAT) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAlphaTypeMap); ++i) {
        if (gAlphaTypeMap[i].fSK == skAT) {
            cAT = gAlphaTypeMap[i].fC;
            return true;
        }
    }
    return false;
}

static bool from_c_info(const sk_imageinfo_t& cinfo, SkImageInfo* info) {
    SkColorType ct;
    SkAlphaType at;

    if (!from_c_colortype(cinfo.colorType, &ct)) {
        // optionally report error to client?
        return false;
    }
    if (!from_c_alphatype(cinfo.alphaType, &at)) {
        // optionally report error to client?
        return false;
    }
    if (info) {
        *info = SkImageInfo::Make(cinfo.width, cinfo.height, ct, at);
    }
    return true;
}

static bool to_c_info(const SkImageInfo &info, sk_imageinfo_t& cinfo) {
    cinfo.width = info.width();
    cinfo.height = info.height();
    if (!to_c_colortype(info.colorType(), &cinfo.colorType) ||
        !to_c_alphatype(info.alphaType(), cinfo.alphaType)) {
        return false;
    }
    return true;
}

static void from_c_matrix(const sk_matrix_t* cmatrix, SkMatrix* matrix) {
    matrix->setAll(cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
                   cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
                   cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
}

const struct {
    sk_path_direction_t fC;
    SkPath::Direction   fSk;
} gPathDirMap[] = {
    { CW_SK_PATH_DIRECTION,  SkPath::kCW_Direction },
    { CCW_SK_PATH_DIRECTION, SkPath::kCCW_Direction },
};

static bool from_c_path_direction(sk_path_direction_t cdir, SkPath::Direction* dir) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gPathDirMap); ++i) {
        if (gPathDirMap[i].fC == cdir) {
            if (dir) {
                *dir = gPathDirMap[i].fSk;
            }
            return true;
        }
    }
    return false;
}

static SkData* AsData(const sk_data_t* cdata) {
    return reinterpret_cast<SkData*>(const_cast<sk_data_t*>(cdata));
}

static sk_data_t* ToData(SkData* data) {
    return reinterpret_cast<sk_data_t*>(data);
}

static sk_rect_t ToRect(const SkRect& rect) {
    return reinterpret_cast<const sk_rect_t&>(rect);
}

static const SkRect& AsRect(const sk_rect_t& crect) {
    return reinterpret_cast<const SkRect&>(crect);
}

static const SkPath& AsPath(const sk_path_t& cpath) {
    return reinterpret_cast<const SkPath&>(cpath);
}

static SkPath* as_path(sk_path_t* cpath) {
    return reinterpret_cast<SkPath*>(cpath);
}

static const SkImage* AsImage(const sk_image_t* cimage) {
    return reinterpret_cast<const SkImage*>(cimage);
}

static sk_image_t* ToImage(SkImage* cimage) {
    return reinterpret_cast<sk_image_t*>(cimage);
}

static sk_canvas_t* ToCanvas(SkCanvas* canvas) {
    return reinterpret_cast<sk_canvas_t*>(canvas);
}

static SkCanvas* AsCanvas(sk_canvas_t* ccanvas) {
    return reinterpret_cast<SkCanvas*>(ccanvas);
}

static SkPictureRecorder* AsPictureRecorder(sk_picture_recorder_t* crec) {
    return reinterpret_cast<SkPictureRecorder*>(crec);
}

static sk_picture_recorder_t* ToPictureRecorder(SkPictureRecorder* rec) {
    return reinterpret_cast<sk_picture_recorder_t*>(rec);
}

static const SkPicture* AsPicture(const sk_picture_t* cpic) {
    return reinterpret_cast<const SkPicture*>(cpic);
}

static SkPicture* AsPicture(sk_picture_t* cpic) {
    return reinterpret_cast<SkPicture*>(cpic);
}

static sk_picture_t* ToPicture(SkPicture* pic) {
    return reinterpret_cast<sk_picture_t*>(pic);
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_colortype_t sk_colortype_get_default_8888() {
    sk_colortype_t ct;
    if (!to_c_colortype(kN32_SkColorType, &ct)) {
        ct = UNKNOWN_SK_COLORTYPE;
    }
    return ct;
}

sk_imageinfo_t sk_imageinfo_make_n32_premul(int width, int height) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(SkISize::Make(width, height));
    sk_imageinfo_t ret;
    const bool convert_res = to_c_info(info, ret);
    SkASSERT(convert_res);
    (void)convert_res;
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_data_t* sk_image_encode(const sk_image_t* cimage) {
    return ToData(AsImage(cimage)->encode());
}

void sk_image_ref(const sk_image_t* cimage) {
    AsImage(cimage)->ref();
}

void sk_image_unref(const sk_image_t* cimage) {
    AsImage(cimage)->unref();
}

int sk_image_get_width(const sk_image_t* cimage) {
    return AsImage(cimage)->width();
}

int sk_image_get_height(const sk_image_t* cimage) {
    return AsImage(cimage)->height();
}

uint32_t sk_image_get_unique_id(const sk_image_t* cimage) {
    return AsImage(cimage)->uniqueID();
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_path_t* sk_path_new() { return (sk_path_t*)new SkPath; }

void sk_path_delete(sk_path_t* cpath) { delete as_path(cpath); }

void sk_path_move_to(sk_path_t* cpath, float x, float y) {
    as_path(cpath)->moveTo(x, y);
}

void sk_path_line_to(sk_path_t* cpath, float x, float y) {
    as_path(cpath)->lineTo(x, y);
}

void sk_path_quad_to(sk_path_t* cpath, float x0, float y0, float x1, float y1) {
    as_path(cpath)->quadTo(x0, y0, x1, y1);
}

void sk_path_conic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float w) {
    as_path(cpath)->conicTo(x0, y0, x1, y1, w);
}

void sk_path_cubic_to(sk_path_t* cpath, float x0, float y0, float x1, float y1, float x2, float y2) {
    as_path(cpath)->cubicTo(x0, y0, x1, y1, x2, y2);
}

void sk_path_close(sk_path_t* cpath) {
    as_path(cpath)->close();
}

void sk_path_add_rect(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!from_c_path_direction(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addRect(AsRect(*crect), dir);
}

void sk_path_add_oval(sk_path_t* cpath, const sk_rect_t* crect, sk_path_direction_t cdir) {
    SkPath::Direction dir;
    if (!from_c_path_direction(cdir, &dir)) {
        return;
    }
    as_path(cpath)->addOval(AsRect(*crect), dir);
}

bool sk_path_get_bounds(const sk_path_t* cpath, sk_rect_t* crect) {
    const SkPath& path = AsPath(*cpath);

    if (path.isEmpty()) {
        if (crect) {
            *crect = ToRect(SkRect::MakeEmpty());
        }
        return false;
    }

    if (crect) {
        *crect = ToRect(path.getBounds());
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

void sk_canvas_save(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->save();
}

void sk_canvas_save_layer(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_restore(sk_canvas_t* ccanvas) {
    AsCanvas(ccanvas)->restore();
}

void sk_canvas_translate(sk_canvas_t* ccanvas, float dx, float dy) {
    AsCanvas(ccanvas)->translate(dx, dy);
}

void sk_canvas_scale(sk_canvas_t* ccanvas, float sx, float sy) {
    AsCanvas(ccanvas)->scale(sx, sy);
}

void sk_canvas_rotate_degrees(sk_canvas_t* ccanvas, float degrees) {
    AsCanvas(ccanvas)->rotate(degrees);
}

void sk_canvas_rotate_radians(sk_canvas_t* ccanvas, float radians) {
    AsCanvas(ccanvas)->rotate(SkRadiansToDegrees(radians));
}

void sk_canvas_skew(sk_canvas_t* ccanvas, float sx, float sy) {
    AsCanvas(ccanvas)->skew(sx, sy);
}

void sk_canvas_concat(sk_canvas_t* ccanvas, const sk_matrix_t* cmatrix) {
    SkASSERT(cmatrix);
    SkMatrix matrix;
    from_c_matrix(cmatrix, &matrix);
    AsCanvas(ccanvas)->concat(matrix);
}

void sk_canvas_clip_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect) {
    AsCanvas(ccanvas)->clipRect(AsRect(*crect));
}

void sk_canvas_clip_path(sk_canvas_t* ccanvas, const sk_path_t* cpath) {
    AsCanvas(ccanvas)->clipPath(AsPath(*cpath));
}

void sk_canvas_draw_paint(sk_canvas_t* ccanvas, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPaint(AsPaint(*cpaint));
}

void sk_canvas_draw_rect(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawRect(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_draw_circle(sk_canvas_t* ccanvas, float cx, float cy, float rad,
                           const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawCircle(cx, cy, rad, AsPaint(*cpaint));
}

void sk_canvas_draw_oval(sk_canvas_t* ccanvas, const sk_rect_t* crect, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawOval(AsRect(*crect), AsPaint(*cpaint));
}

void sk_canvas_draw_path(sk_canvas_t* ccanvas, const sk_path_t* cpath, const sk_paint_t* cpaint) {
    AsCanvas(ccanvas)->drawPath(AsPath(*cpath), AsPaint(*cpaint));
}

void sk_canvas_draw_color(sk_canvas_t *ccanvas, uint32_t color)
{
    AsCanvas(ccanvas)->drawColor(color);
}

void sk_canvas_draw_text(sk_canvas_t *ccanvas, const void* text, size_t byteLength, float x, float y, const sk_paint_t *cpaint)
{
    AsCanvas(ccanvas)->drawText(text, byteLength, x, y, *AsPaint(cpaint));
}

void sk_canvas_draw_line(sk_canvas_t *ccanvas, float x0, float y0, float x1, float y1, const sk_paint_t *cpaint)
{
    AsCanvas(ccanvas)->drawLine(x0, y0, x1, y1, *AsPaint(cpaint));
}

void sk_canvas_draw_arc(sk_canvas_t *ccanvas, const sk_rect_t *coval, float start_angle,
			float sweep_angle, bool use_center, const sk_paint_t *cpaint)
{
    AsCanvas(ccanvas)->drawArc(AsRect(*coval), start_angle, sweep_angle, use_center, *AsPaint(cpaint));
}

///////////////////////////////////////////////////////////////////////////////////////////


sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t* cinfo, void* pixels,
                                           size_t rowBytes,
                                           const sk_surfaceprops_t* props) {
    SkImageInfo info;
    if (!from_c_info(*cinfo, &info)) {
        return NULL;
    }
    
    return (sk_surface_t*)SkSurface::NewRasterDirect(info, pixels, rowBytes);
}

void sk_surface_unref(sk_surface_t* csurf) {
    SkSafeUnref((SkSurface*)csurf);
}

sk_canvas_t* sk_surface_get_canvas(sk_surface_t* csurf) {
    SkSurface* surf = (SkSurface*)csurf;
    return (sk_canvas_t*)surf->getCanvas();
}

int sk_surface_width(const sk_surface_t* csurf) {
  const SkSurface* surf = (const SkSurface*)csurf;
  return surf->width();
}

int sk_surface_height(const sk_surface_t* csurf) {
  const SkSurface* surf = (const SkSurface*)csurf;
  return surf->height();
}

///////////////////////////////////////////////////////////////////////////////////////////

sk_picture_recorder_t* sk_picture_recorder_new() {
    return ToPictureRecorder(new SkPictureRecorder);
}

void sk_picture_recorder_delete(sk_picture_recorder_t* crec) {
    delete AsPictureRecorder(crec);
}

void sk_picture_ref(sk_picture_t* cpic) {
    SkSafeRef(AsPicture(cpic));
}

void sk_picture_unref(sk_picture_t* cpic) {
    SkSafeUnref(AsPicture(cpic));
}

uint32_t sk_picture_get_unique_id(sk_picture_t* cpic) {
    return AsPicture(cpic)->uniqueID();
}

///////////////////////////////////////////////////////////////////////////////////////////

void sk_data_ref(const sk_data_t* cdata) {
    SkSafeRef(AsData(cdata));
}

void sk_data_unref(const sk_data_t* cdata) {
    SkSafeUnref(AsData(cdata));
}

size_t sk_data_get_size(const sk_data_t* cdata) {
    return AsData(cdata)->size();
}

const void* sk_data_get_data(const sk_data_t* cdata) {
    return AsData(cdata)->data();
}

///////////////////////////////////////////////////////////////////////////////////////////
