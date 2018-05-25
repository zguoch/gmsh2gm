// Gmsh - Copyright (C) 1997-2018 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@onelab.info>.

#include <algorithm>
#include "MEdge.h"
#include "Numeric.h"
#include "GmshDefines.h"
#include "ElementType.h"
#include "nodalBasis.h"
#include "BasisFactory.h"

// FIXME
// remove that when MElementCut is removed
bool MEdge::isInside(MVertex *v) const
{
  double tol = MVertexLessThanLexicographic::getTolerance();
  MVertex *v0 = _v[0];
  MVertex *v1 = _v[1];
  MVertexLessThanLexicographic lt;
  if(lt(v0, v1)){
    v0 = _v[1]; v1 = _v[0];
  }
  double x = v->x(), y = v->y(), z = v->z();
  double x0 = v0->x(), y0 = v0->y(), z0 = v0->z();
  double x1 = v1->x(), y1 = v1->y(), z1 = v1->z();
  if(fabs(x - x0) < tol && fabs(y - y0) < tol && fabs(z - z0) < tol)
    return true;
  if(fabs(x - x1) < tol && fabs(y - y1) < tol && fabs(z - z1) < tol)
    return true;
  if(x < x0 - tol || x > x1 + tol ||
     y < std::min(y0, y1) - tol || y > std::max(y0, y1) + tol ||
     z < std::min(z0, z1) - tol || z > std::max(z0, z1) + tol)
    return false;
  if(fabs(x1 - x0) > tol){
    double tx = (x - x0) / (x1 - x0);
    if(fabs(y1 - y0) > tol){
      double ty = (y - y0) / (y1 - y0);
      if(fabs(z1 - z0) > tol){
        double tz = (z - z0) / (z1 - z0);
        if(fabs(tx - ty) > tol || fabs(tx - tz) > tol)
          return false;
      }
      else{
        if(fabs(tx - ty) > tol)
          return false;
      }
    }
    else{
      if(fabs(z1 - z0) > tol){
        double tz = (z - z0) / (z1 - z0);
        if(fabs(tx - tz) > tol)
          return false;
      }
    }
  }
  else{
    if(fabs(y1 - y0) > tol){
      double ty = (y - y0) / (y1 - y0);
      if(fabs(z1 - z0) > tol){
        double tz = (z - z0) / (z1 - z0);
        if(fabs(ty - tz) > tol)
          return false;
      }
    }
  }
  return true;
}


bool SortEdgeConsecutive(const std::vector<MEdge> &e,
                         std::vector<std::vector<MVertex*> >&vs)
{
  if(e.empty()) return true;
  std::map<MVertex*, std::pair<MVertex*, MVertex*> > c;
  for (size_t i = 0; i<e.size();i++){
    MVertex *v0 = e[i].getVertex(0);
    MVertex *v1 = e[i].getVertex(1);
    std::map<MVertex*, std::pair<MVertex*,MVertex*> >::iterator it0 = c.find(v0);
    std::map<MVertex*, std::pair<MVertex*,MVertex*> >::iterator it1 = c.find(v1);
    if (it0 == c.end()) c[v0] = std::make_pair(v1,(MVertex*)NULL);
    else it0->second.second = v1;
    if (it1 == c.end()) c[v1] = std::make_pair(v0,(MVertex*)NULL);
    else it1->second.second = v0;
  }

  while (!c.empty()){
    std::vector<MVertex*> v;
    MVertex *start = NULL;
    {
      std::map<MVertex*, std::pair<MVertex*,MVertex*> >::iterator it = c.begin();
      start = it->first;
      for (; it != c.end(); ++it) {
        if (it->second.second == NULL){
          start = it->first;
          break;
        }
      }
    }
    std::map<MVertex*, std::pair<MVertex*,MVertex*> >::iterator it = c.find(start);
    MVertex *prev    = (it->second.second == start) ? it->second.first : it->second.second;
    MVertex *current = start;
    do {
      v.push_back(current);
      std::map<MVertex*, std::pair<MVertex*,MVertex*> >::iterator it = c.find(current);
      c.erase(it);
      MVertex *v1 = it->second.first;
      MVertex *v2 = it->second.second;
      MVertex *temp = current;
      if (v1 == prev)current = v2;
      else if (v2 == prev)current = v1;
      else {
        break;
      }
      prev = temp;
      if (current == start) {
        v.push_back(current);
      }
    } while (current != start && current != NULL);
    vs.push_back(v);
  }

  return true;
}

MEdgeN::MEdgeN(const std::vector<MVertex*> &v)
{
  _v.resize(v.size());
  for(unsigned int i = 0; i < v.size(); i++)
    _v[i] = v[i];
}

MEdge MEdgeN::getEdge() const
{
  return MEdge(_v[0], _v[1]);
}

SPoint3 MEdgeN::pnt(double u) const
{
  int tagLine = ElementType::getTag(TYPE_LIN, (int)_v.size() - 1);
  const nodalBasis *fs = BasisFactory::getNodalBasis(tagLine);

  double f[100];
  fs->f(u, 0, 0, f);

  double x = 0, y = 0, z = 0;
  for (int j = 0; j < fs->getNumShapeFunctions(); j++) {
    x += f[j] * _v[j]->x();
    y += f[j] * _v[j]->y();
    z += f[j] * _v[j]->z();
  }
  return SPoint3(x, y, z);
}

SVector3 MEdgeN::tangent(double u) const
{
  int tagLine = ElementType::getTag(TYPE_LIN, (int)_v.size() - 1);
  const nodalBasis *fs = BasisFactory::getNodalBasis(tagLine);

  double sf[100][3];
  fs->df(u, 0, 0, sf);

  double dx = 0, dy = 0, dz = 0;
  for (int j = 0; j < fs->getNumShapeFunctions(); j++) {
    dx += sf[j][0] * _v[j]->x();
    dy += sf[j][0] * _v[j]->y();
    dz += sf[j][0] * _v[j]->z();
  }
  return SVector3(dx, dy, dz).unit();
}
