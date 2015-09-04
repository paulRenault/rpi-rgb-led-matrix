// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
// Copyright (C) 2015 Christoph Friedrich <christoph.friedrich@vonaffenfels.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#ifndef RPI_TRANSFORMER_H
#define RPI_TRANSFORMER_H

#include <vector>
#include <cstddef>

#include "canvas.h"
enum led_matrix_orientation {HORIZONTAL=0,VERTICAL=270,VERTICAL_UPSIDE_DOWN=90,HORIZONTAL_UPSIDE_DOWN=180};
namespace rgb_matrix {

// Transformer for RotateCanvas
class RotateTransformer : public CanvasTransformer {
public:
  RotateTransformer(led_matrix_orientation angle = HORIZONTAL);
  virtual ~RotateTransformer();

  void SetAngle(led_matrix_orientation angle);
  inline led_matrix_orientation angle() { return angle_; }

  virtual Canvas *Transform(Canvas *output);

private:
  // Transformer canvas to rotate the input canvas in 90° steps
  class TransformCanvas;

  led_matrix_orientation angle_;
  TransformCanvas *const canvas_;
};

// Transformer for linked transformer objects
// First transformer added will be considered last
// (so it would the transformer that gets the original Canvas object)
class LinkedTransformer : public CanvasTransformer {
public:
  typedef std::vector<CanvasTransformer*> List;

  LinkedTransformer() {}
  LinkedTransformer(List transformer_list) : list_(transformer_list) {}

  // The ownership of the given transformers is _not_ taken over unless
  // you explicitly call DeleteTransformers().
  void AddTransformer(CanvasTransformer *transformer);
  void AddTransformer(List transformer_list);
  void SetTransformer(List transformer_list);

  // Delete transformers that have been added or set.
  void DeleteTransformers();

  // -- CanvasTransformer interface
  virtual Canvas *Transform(Canvas *output);

private:
  List list_;
};

class LargeSquare64x64Transformer : public CanvasTransformer {
public:
  LargeSquare64x64Transformer();
  virtual ~LargeSquare64x64Transformer();

  virtual Canvas *Transform(Canvas *output);

private:
  class TransformCanvas;

  TransformCanvas *const canvas_;
};

} // namespace rgb_matrix

#endif // RPI_TRANSFORMER_H
