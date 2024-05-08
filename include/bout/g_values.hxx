
#ifndef BOUT_GVALUES_HXX
#define BOUT_GVALUES_HXX

#include "spatial_dimensions.hxx"

using FieldMetric = SpatialDimensions::FieldMetric;

/// `GValues` needs renaming, when we know what the name should be
class GValues {

public:
  GValues(FieldMetric G1, FieldMetric G2, FieldMetric G3);

  explicit GValues(Coordinates& coordinates);

  FieldMetric& G1() { return G1_; }
  FieldMetric& G2() { return G2_; }
  FieldMetric& G3() { return G3_; }

  void setG1(const FieldMetric& G1) { G1_ = G1; }
  void setG2(const FieldMetric& G2) { G2_ = G2; }
  void setG3(const FieldMetric& G3) { G3_ = G3; }

private:
  FieldMetric G1_, G2_, G3_;
};

#endif //BOUT_GVALUES_HXX
