
#ifndef BOUT_METRIC_TENSOR_HXX
#define BOUT_METRIC_TENSOR_HXX

#include "spatial_dimensions.hxx"
#include "bout/field2d.hxx"
#include "bout/field3d.hxx"
#include <bout/bout_types.hxx>
#include <functional>

using FieldMetric = SpatialDimensions::FieldMetric;

class MetricTensor {

protected:

  MetricTensor(FieldMetric g11, FieldMetric g22, FieldMetric g33, FieldMetric g12,
               FieldMetric g13, FieldMetric g23);

  MetricTensor(BoutReal g11, BoutReal g22, BoutReal g33, BoutReal g12, BoutReal g13,
               BoutReal g23, Mesh* mesh);

public:

  FieldMetric& g11() { return g11_; }
  FieldMetric& g22() { return g22_; }
  FieldMetric& g33() { return g33_; }
  FieldMetric& g12() { return g12_; }
  FieldMetric& g13() { return g13_; }
  FieldMetric& g23() { return g23_; }

  void setMetricTensor(MetricTensor& metric_tensor) {

    g11_ = metric_tensor.g11();
    g22_ = metric_tensor.g22();
    g33_ = metric_tensor.g33();
    g12_ = metric_tensor.g12();
    g13_ = metric_tensor.g13();
    g23_ = metric_tensor.g23();
  }

  void setLocation(const CELL_LOC location) {
    g11_.setLocation(location);
    g22_.setLocation(location);
    g33_.setLocation(location);
    g12_.setLocation(location);
    g13_.setLocation(location);
    g23_.setLocation(location);
  }

  MetricTensor inverse(const std::string& region = "RGN_ALL");

  // check that tensors are positive (if expected) and finite (always)
  void check(int ystart);

  // Transforms the MetricTensor by applying the given function to every component
  void map(const std::function<const FieldMetric(const FieldMetric)>& function);

private:

  FieldMetric g11_, g22_, g33_, g12_, g13_, g23_;

  MetricTensor applyToComponents(
      const std::function<const FieldMetric(const FieldMetric)>& function) const;
};

class CovariantMetricTensor : public MetricTensor {

public:
  CovariantMetricTensor(FieldMetric g11, FieldMetric g22, FieldMetric g33,
                        FieldMetric g12, FieldMetric g13, FieldMetric g23)
      : MetricTensor(std::move(g11), std::move(g22), std::move(g33), std::move(g12),
                     std::move(g13), std::move(g23)){};

  CovariantMetricTensor(const BoutReal g11, const BoutReal g22, const BoutReal g33,
                        const BoutReal g12, const BoutReal g13, const BoutReal g23,
                        Mesh* mesh)
      : MetricTensor(g11, g22, g33, g12, g13, g23, mesh){};
};

class ContravariantMetricTensor : public MetricTensor {

public:
  ContravariantMetricTensor(FieldMetric g_11, FieldMetric g_22, FieldMetric g_33,
                            FieldMetric g_12, FieldMetric g_13, FieldMetric g_23)
      : MetricTensor(std::move(g_11), std::move(g_22), std::move(g_33), std::move(g_12),
                     std::move(g_13), std::move(g_23)){};

  ContravariantMetricTensor(const BoutReal g_11, const BoutReal g_22, const BoutReal g_33,
                            const BoutReal g_12, const BoutReal g_13, const BoutReal g_23,
                            Mesh* mesh)
      : MetricTensor(g_11, g_22, g_33, g_12, g_13, g_23, mesh){};
};

#endif //BOUT_METRIC_TENSOR_HXX
