
#include "bout/ContravariantMetricTensor.hxx"
#include "bout/CovariantMetricTensor.hxx"

ContravariantMetricTensor::ContravariantMetricTensor(
    const FieldMetric& g11, const FieldMetric& g22, const FieldMetric& g33,
    const FieldMetric& g12, const FieldMetric& g13, const FieldMetric& g23)
    : g11(std::move(g11)), g22(std::move(g22)), g33(std::move(g33)), g12(std::move(g12)),
      g13(std::move(g13)), g23(std::move(g23)) {

  Allocate(); // Make sure metric elements are allocated //  ; TODO: Required?
}

ContravariantMetricTensor::ContravariantMetricTensor(
    const BoutReal g11, const BoutReal g22, const BoutReal g33, const BoutReal g12,
    const BoutReal g13, const BoutReal g23, Mesh* mesh)
    : g11(g11, mesh), g22(g22, mesh), g33(g33, mesh), g12(g12, mesh), g13(g13, mesh),
      g23(g23, mesh) {

  Allocate(); // Make sure metric elements are allocated //  ; TODO: Required?
}

const ContravariantMetricTensor::FieldMetric& ContravariantMetricTensor::Getg11() const {
  return g11;
}
const ContravariantMetricTensor::FieldMetric& ContravariantMetricTensor::Getg22() const {
  return g22;
}
const ContravariantMetricTensor::FieldMetric& ContravariantMetricTensor::Getg33() const {
  return g33;
}
const ContravariantMetricTensor::FieldMetric& ContravariantMetricTensor::Getg12() const {
  return g12;
}
const ContravariantMetricTensor::FieldMetric& ContravariantMetricTensor::Getg13() const {
  return g13;
}
const ContravariantMetricTensor::FieldMetric& ContravariantMetricTensor::Getg23() const {
  return g23;
}

void ContravariantMetricTensor::setContravariantMetricTensor(
    const ContravariantMetricTensor& metric_tensor) {

  g11 = metric_tensor.Getg11();
  g22 = metric_tensor.Getg22();
  g33 = metric_tensor.Getg33();
  g12 = metric_tensor.Getg12();
  g13 = metric_tensor.Getg13();
  g23 = metric_tensor.Getg23();
}

void ContravariantMetricTensor::checkContravariant(int ystart) {
  // Diagonal metric components should be finite
  bout::checkFinite(g11, "g11", "RGN_NOCORNERS");
  bout::checkFinite(g22, "g22", "RGN_NOCORNERS");
  bout::checkFinite(g33, "g33", "RGN_NOCORNERS");
  if (g11.hasParallelSlices() && &g11.ynext(1) != &g11) {
    for (int dy = 1; dy <= ystart; ++dy) {
      for (const auto sign : {1, -1}) {
        bout::checkFinite(g11.ynext(sign * dy), "g11.ynext",
                          fmt::format("RGN_YPAR_{:+d}", sign * dy));
        bout::checkFinite(g22.ynext(sign * dy), "g22.ynext",
                          fmt::format("RGN_YPAR_{:+d}", sign * dy));
        bout::checkFinite(g33.ynext(sign * dy), "g33.ynext",
                          fmt::format("RGN_YPAR_{:+d}", sign * dy));
      }
    }
  }
  // Diagonal metric components should be positive
  bout::checkPositive(g11, "g11", "RGN_NOCORNERS");
  bout::checkPositive(g22, "g22", "RGN_NOCORNERS");
  bout::checkPositive(g33, "g33", "RGN_NOCORNERS");
  if (g11.hasParallelSlices() && &g11.ynext(1) != &g11) {
    for (int dy = 1; dy <= ystart; ++dy) {
      for (const auto sign : {1, -1}) {
        bout::checkPositive(g11.ynext(sign * dy), "g11.ynext",
                            fmt::format("RGN_YPAR_{:+d}", sign * dy));
        bout::checkPositive(g22.ynext(sign * dy), "g22.ynext",
                            fmt::format("RGN_YPAR_{:+d}", sign * dy));
        bout::checkPositive(g33.ynext(sign * dy), "g33.ynext",
                            fmt::format("RGN_YPAR_{:+d}", sign * dy));
      }
    }
  }

  // Off-diagonal metric components should be finite
  bout::checkFinite(g12, "g12", "RGN_NOCORNERS");
  bout::checkFinite(g13, "g13", "RGN_NOCORNERS");
  bout::checkFinite(g23, "g23", "RGN_NOCORNERS");
  if (g23.hasParallelSlices() && &g23.ynext(1) != &g23) {
    for (int dy = 1; dy <= ystart; ++dy) {
      for (const auto sign : {1, -1}) {
        bout::checkFinite(g12.ynext(sign * dy), "g12.ynext",
                          fmt::format("RGN_YPAR_{:+d}", sign * dy));
        bout::checkFinite(g13.ynext(sign * dy), "g13.ynext",
                          fmt::format("RGN_YPAR_{:+d}", sign * dy));
        bout::checkFinite(g23.ynext(sign * dy), "g23.ynext",
                          fmt::format("RGN_YPAR_{:+d}", sign * dy));
      }
    }
  }
}

void ContravariantMetricTensor::Allocate() { //  ; TODO: Required?
  g11.allocate();
  g22.allocate();
  g33.allocate();
  g12.allocate();
  g13.allocate();
  g23.allocate();
}

void ContravariantMetricTensor::setLocation(const CELL_LOC location) {
  g11.setLocation(location);
  g22.setLocation(location);
  g33.setLocation(location);
  g12.setLocation(location);
  g13.setLocation(location);
  g23.setLocation(location);
}

void ContravariantMetricTensor::calcContravariant(
    CovariantMetricTensor covariantMetricTensor, CELL_LOC location,
    const std::string& region) {

  TRACE("ContravariantMetricTensor::calcContravariant");

  // Perform inversion of g_{ij} to get g^{ij}
  // NOTE: Currently this bit assumes that metric terms are Field2D objects

  auto a = Matrix<BoutReal>(3, 3);

  BOUT_FOR_SERIAL(i, covariantMetricTensor.Getg_11().getRegion(region)) {
    a(0, 0) = covariantMetricTensor.Getg_11()[i];
    a(1, 1) = covariantMetricTensor.Getg_22()[i];
    a(2, 2) = covariantMetricTensor.Getg_33()[i];

    a(0, 1) = a(1, 0) = covariantMetricTensor.Getg_12()[i];
    a(1, 2) = a(2, 1) = covariantMetricTensor.Getg_23()[i];
    a(0, 2) = a(2, 0) = covariantMetricTensor.Getg_13()[i];

    if (invert3x3(a)) {
      const auto error_message = "\tERROR: metric tensor is singular at ({:d}, {:d})\n";
      output_error.write(error_message, i.x(), i.y());
      throw BoutException(error_message);
    }
  }

  g11 = a(0, 0);
  g22 = a(1, 1);
  g33 = a(2, 2);
  g12 = a(0, 1);
  g13 = a(0, 2);
  g23 = a(1, 2);
  //  contravariant_components = ContravariantComponents{a(0, 0), a(1, 1), a(2, 2), a(0, 1), a(0, 2), a(1, 2)};

  setLocation(location);

  BoutReal maxerr;
  maxerr = BOUTMAX(max(abs((covariantMetricTensor.Getg_11() * g11
                            + covariantMetricTensor.Getg_12() * g12
                            + covariantMetricTensor.Getg_13() * g13)
                           - 1)),
                   max(abs((covariantMetricTensor.Getg_12() * g12
                            + covariantMetricTensor.Getg_22() * g22
                            + covariantMetricTensor.Getg_23() * g23)
                           - 1)),
                   max(abs((covariantMetricTensor.Getg_13() * g13
                            + covariantMetricTensor.Getg_23() * g23
                            + covariantMetricTensor.Getg_33() * g33)
                           - 1)));

  output_info.write("\tMaximum error in diagonal inversion is {:e}\n", maxerr);

  maxerr = BOUTMAX(max(abs(covariantMetricTensor.Getg_11() * g12
                           + covariantMetricTensor.Getg_12() * g22
                           + covariantMetricTensor.Getg_13() * g23)),
                   max(abs(covariantMetricTensor.Getg_11() * g13
                           + covariantMetricTensor.Getg_12() * g23
                           + covariantMetricTensor.Getg_13() * g33)),
                   max(abs(covariantMetricTensor.Getg_12() * g13
                           + covariantMetricTensor.Getg_22() * g23
                           + covariantMetricTensor.Getg_23() * g33)));

  output_info.write("\tMaximum error in off-diagonal inversion is {:e}\n", maxerr);
}
