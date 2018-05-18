/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_types_priv.h"
#include "SkMatrix.h"

static void from_c_matrix(const sk_matrix_t* cmatrix, SkMatrix* matrix) {
    matrix->setAll(cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
                   cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
                   cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
}

#include "../../include/effects/SkGradientShader.h"
#include "sk_shader.h"

const struct {
    sk_shader_tilemode_t    fC;
    SkShader::TileMode      fSK;
} gTileModeMap[] = {
    { CLAMP_SK_SHADER_TILEMODE,     SkShader::kClamp_TileMode },
    { REPEAT_SK_SHADER_TILEMODE,    SkShader::kRepeat_TileMode },
    { MIRROR_SK_SHADER_TILEMODE,    SkShader::kMirror_TileMode  },
};

static bool from_c_tilemode(sk_shader_tilemode_t cMode, SkShader::TileMode* skMode) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTileModeMap); ++i) {
        if (cMode == gTileModeMap[i].fC) {
            if (skMode) {
                *skMode = gTileModeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

void sk_shader_ref(sk_shader_t* cshader) {
    SkSafeRef(AsShader(cshader));
}

void sk_shader_unref(sk_shader_t* cshader) {
    SkSafeUnref(AsShader(cshader));
}


static const SkPoint& to_skpoint(const sk_point_t& p) {
    return reinterpret_cast<const SkPoint&>(p);
}



///////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/effects/SkBlurMaskFilter.h"
#include "sk_maskfilter.h"

const struct {
    sk_blurstyle_t  fC;
    SkBlurStyle     fSk;
} gBlurStylePairs[] = {
    { NORMAL_SK_BLUR_STYLE, kNormal_SkBlurStyle },
    { SOLID_SK_BLUR_STYLE,  kSolid_SkBlurStyle },
    { OUTER_SK_BLUR_STYLE,  kOuter_SkBlurStyle },
    { INNER_SK_BLUR_STYLE,  kInner_SkBlurStyle },
};

static bool find_blurstyle(sk_blurstyle_t csrc, SkBlurStyle* dst) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gBlurStylePairs); ++i) {
        if (gBlurStylePairs[i].fC == csrc) {
            if (dst) {
                *dst = gBlurStylePairs[i].fSk;
            }
            return true;
        }
    }
    return false;
}

void sk_maskfilter_ref(sk_maskfilter_t* cfilter) {
    SkSafeRef(AsMaskFilter(cfilter));
}

void sk_maskfilter_unref(sk_maskfilter_t* cfilter) {
    SkSafeUnref(AsMaskFilter(cfilter));
}

