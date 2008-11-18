// Gmsh - Copyright (C) 1997-2008 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _HIGH_ORDER_H_
#define _HIGH_ORDER_H_

#include "GModel.h"

void SetOrder1(GModel *m);
void SetOrderN(GModel *m, int order, bool linear=true, bool incomplete=false);
void checkHighOrderTriangles(GModel *m);
bool reparamOnFace(MVertex *v, GFace *gf, SPoint2 &param);

void Refine(GModel *m, bool linear=true);

#endif
